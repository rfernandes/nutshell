#include "Shell.h"

#include <command/Command.h>
#include <shell/History.h>
#include <shell/Utils.h>

#include <algorithm>
#include <experimental/string_view>
#include <fstream>

using namespace std;
using namespace std::experimental;
using namespace manip;

class BuiltIn: public Command
{
  bool matches(const Line& line) const{
    return line.substr(0, line.find_first_of(' ')) == _command;
  }

public:
  using Function = function<Status(const Line&, Output&)>;

  BuiltIn(string command, Function function)
  : _command{move(command)}
  , _function{move(function)}
  {
  }

  ~BuiltIn() override = default;

  Description parse(const Line& line, Output& output, bool execute) override {
    auto matched = matches(line);
    if (matched && execute){
      _function(line, output);
    }

    Description desc;
    if (matched){
      desc.status = Status::Ok;
      desc.segments.push_back({Segment::Type::Builtin, string_view{line}});
    }

    return desc;
  }

private:
  const string _command;
  const Function _function;
};

Shell::Shell()
: _store{CommandStore::instance()}
, _history{CommandStore::store<History>()}
, _out{cout}
, _cd{CommandStore::store<Cd>()}
, _function{
    CommandStore::store<Function>(std::unordered_map<string, string>{
    { ":prompt",
      "\"\x1b[38;2;230;120;150mNutshell\x1b[0m├─┤\x1b[38;2;120;150;230m\"\n"
      ":cwd\n"
      "\"\x1b[0m│ \""},
    { ":prompt2",
      "date"},
    { ":prompt_feed",
      "\"feed: \""}
    })}
, _exit{false}
, _utf8Bytes{0}
, _match{nullptr}
, _predictive{true}
, _assistive{false}
{
  setlocale(LC_ALL, "");

  CommandStore::store<BuiltIn>(":exit",
                               [&](const Line& /*line*/, Output& /*output*/){
                                 _exit = true;
                                 return Status::Ok;
                               });

  CommandStore::store<BuiltIn>(":help",
                               [](const Line& /*line*/, Output& output){
                                 output << "No one can help you (for now)\n";
                                 return Status::Ok;
                               });

  CommandStore::store<BuiltIn>(":cwd",
                               [&](const Line& /*line*/, Output& output){
                                 output << _cd.cwd().string();
                                 return Status::Ok;
                               });

  // Comments are a simple no-op
  CommandStore::store<BuiltIn>("#",
                               [](const Line& /*line*/, Output& /*output*/){
                                 return Status::Ok;
                               });

  // Toggle predictive functionality
  CommandStore::store<BuiltIn>(":predictive",
                               [&](const Line& /*line*/, Output& /*output*/){
                                 _predictive = !_predictive;
                                 return Status::Ok;
                               });

  // Toggle assistive functionality
  CommandStore::store<BuiltIn>(":assistive",
                               [&](const Line& /*line*/, Output& /*output*/){
                                 _assistive = !_assistive;
                                 return Status::Ok;
                               });

  // Source ~/nutshellrc
  ifstream config{_cd.home() / ".nutshellrc"};
  if (config) {
    string command;
    while (getline(config, command)) {
      _store.parse(command, _out, true);
    }
  }
}

/* FIXME: Perhaps Line should be included in Description
*  TODO:
*    Cleanup single suggestion when deleting last match
*    Clear &/Pad when executed
*    Compact view (maximise line information)
*/
ostream & assist(ostream &out, const Suggestion::const_iterator &start, const Description &description, size_t idx){
  auto it = start;
  for (size_t i = 0; i < idx; ++i) {
    const auto &segment = description.segments.at(i);
    const auto start = distance(it, segment.view.begin());
    it = segment.view.begin();
    fill_n(ostreambuf_iterator<char>(out), start, ' ');
    if (i + 1 == idx){

      switch (segment.type){
        case Segment::Type::Builtin:
          out << Color::Cyan;
          break;
        case Segment::Type::Command:
          out << Color::Green;
          break;
        case Segment::Type::Parameter:
          out << Color::Blue;
          break;
        case Segment::Type::Argument:
          out << Color::Magenta;
          break;
        case Segment::Type::String:
          out << Mode::Bold << Color::Blue;
          break;
        default:
          out << Color::Yellow;
      }
      out << "╰╸last";
    }else{
      out << "│" << Color::Reset;
      ++it;
    }
  }

  if (idx > 0){
    out << "\n";
    assist(out, start, description, idx - 1);
  }
  return out;
}

void Shell::line() {
  _cursor.column(_column);
  auto matched = _store.parse(_line, _out, false);
  if (matched.status != Status::NoMatch){
    const auto view = string_view{_line};
    auto it = view.begin();
    for (const auto& segment: matched.segments){
      if (it != segment.view.begin()){
        _out << view.substr(it - view.begin(), distance(it, segment.view.begin()));
      }
      it = segment.view.end();

      switch (segment.type){
        case Segment::Type::Builtin:
          _out << Color::Cyan;
          break;
        case Segment::Type::Command:
          _out << Color::Green;
          break;
        case Segment::Type::Parameter:
          _out << Color::Blue;
          break;
        case Segment::Type::Argument:
          _out << Color::Magenta;
          break;
        case Segment::Type::String:
          _out << Mode::Bold << Color::Blue;
          break;
        default:
          _out << Color::Yellow;
      }
      _out << segment.view;
    }
    _out << Color::Reset;
    if (it != view.end()){
      _out << view.substr(it - view.begin());
    }

    // Assistive (description)
    if (_assistive){
      _cursor.save();
      auto column = _cursor.position().x;

      auto segments = matched.segments.size();
      stringstream block;
      assist(block, view.begin(), matched, segments);

      auto lastLine{false};

      string line;
      while (getline(block, line)){
        _cursor.forceDown();
        lastLine |= _cursor.max().y == _cursor.position().y;
        _cursor.column(_column);
        _out << line << Color::Reset << Erase::CursorToEnd;
      }

      if (lastLine ){
        _cursor.up(segments);
        _cursor.column(column);
      }else{
        _out << "\n" << Erase::CursorToEnd;
        _cursor.restore();
      }
    }

  }else{
    _out << Color::Red << _line << Color::Reset;
  }

  // Update suggestions
  if (_predictive){
    _suggestion = _history.suggest(_line);
    if (!_suggestion.empty()){
      _out << _suggestion.substr(_line.size()) << Color::Reset;
    }
    _out << Erase::CursorToEnd;
  }
}

void Shell::prompt() {
  // call Function "directly", instead of going through store
  _function.parse(":prompt", _out, true);
  _column = _cursor.position().x;

  _idx = 0;
}


int Shell::run() {
  prompt();

  Line buffer;

  unsigned keystroke;
  while (!_exit && (keystroke = _in.get())) {
    switch (keystroke) {
      case Input::CtrlM:
        if (!_suggestion.empty()){
          _line = _suggestion.to_string();
          _suggestion = Suggestion{};
        }
      case '\n':
        _out << '\n';

        buffer += _line;

        if (!buffer.empty()) {
          const auto startTime = std::chrono::system_clock::now();
          try{
            const Description executionResult {
              _match ? _match->parse(buffer, _out, true)
                     : _store.parse(buffer, _out, true)};
            switch (executionResult.status) {
              case Status::Ok:
                break;
              case Status::NoMatch:
                _out << "Command not found [" << _line.substr(0, _line.find_first_of(' ')) << "]\n";
                break;
              case Status::Incomplete:
                _function.parse(":prompt_feed", _out, true);
                _column = _cursor.position().x;
                _idx = 0;
                buffer += "\n";
                _line.clear();
                _suggestion = Suggestion{};
                continue;
            }
          } catch (exception& ex) {
            _out << "Error " << ex.what() << "\n";
          }
          const auto endTime = std::chrono::system_clock::now();
          _history.add({buffer, 0, startTime, endTime});
          buffer.clear();
          _line.clear();
          _suggestion = Suggestion{};
        }
        prompt();
        break;
      case Input::Backspace:
      case '\b': // Ctrl-H
        if (!_idx){
          break;
        }
        _cursor.left();
        while (!utf8::is_utf8(_line[--_idx])){};
      case Input::Delete: {
        _line.erase(_idx, utf8::bytes(_line[_idx]));
        line();
        _cursor.column(utf8::idx(_line, _idx) + _column);
        break;
      }
      case Input::Left:
        if (_idx > 0){
          _cursor.left();
          while (!utf8::is_utf8(_line[--_idx])){}
        }
        break;
      case Input::Right:
        if (_line[_idx]){
          _cursor.right();
          _idx += utf8::bytes(_line[_idx]);;
        }
        break;
      case Input::Up:
      case Input::Down: {
        _line = keystroke == Input::Down ? _history.forward(_line) : _history.backward(_line);
        line();
        _idx = utf8::size(_line);
        break;
      }
      case Input::Home:
      case Input::End:
        _idx = _line.size();
        _cursor.column(keystroke == Input::Home ? _column: _column + utf8::size(_line));
        break;
      case 18: //Ctrl-R
        _out << "History interactive mode unimplemented\n";
        break;
      case 23:{ //Ctrl-W
        if (!_idx){
          break;
        }
        auto start = _idx;
        while (--_idx && _line[_idx] == ' '){};
        while (_idx-- && _line[_idx] != ' '){};
        if (_idx != 0) ++_idx;
        _line.erase(_idx, start - _idx);
        line();
        _cursor.column(utf8::idx(_line, _idx) + _column);
        break;
      }
      case 4: //Ctrl-D
        _exit = true;
        break;
      case Input::Unknown: // Unknown special key received
        break; // ignore list
      default: {
        if (keystroke > 0xff) {
          _out << "■";
          continue;
        }
        _line.insert(_idx++, 1, keystroke);
        if (!_utf8Bytes){
          _utf8Bytes += utf8::bytes(keystroke);
        }
        if (!--_utf8Bytes){
          line();
          _cursor.column(utf8::idx(_line, _idx) + _column);
        }
        break;
      }
    }
  }
  return 0;
}

