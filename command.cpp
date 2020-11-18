/*
 * MIT License
 * 
 * opyright (c) 2020 jimi36
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
#include "command.h"

#include <stdarg.h>

namespace easycmd {

    namespace internal
    {
        bool is_command_arg(const char *arg) {
            int len = (int)strlen(arg);
            for (int i = 0; i < len; i++) {
                if (isalpha(arg[i++]) == 0) {
                    return false;
                }
            }
            return true;
        }

        bool is_option_arg(const char *arg){
            if (strlen(arg) < 2 || 
                strncmp(arg, "-", 1) != 0) {
                return false;
            }
            return true;
        }

        bool is_long_option_arg(const char *arg) {
            if (strlen(arg) < 3 ||
                strncmp(arg, "--", 2) != 0 || 
                isalpha(arg[2]) == 0) {
                return false;
            }
            return true;
        }

        bool is_short_option_arg(const char *arg) {
            if (strlen(arg) < 2 ||
                strncmp(arg, "-", 1) != 0 ||
                isalpha(arg[1]) == 0) {
                return false;
            }
            return true;
        }

        bool is_short_option(const char c) {
            if (isalpha(c) == 0) {
                return false;
            }
            return true;
        }

        bool is_int_value(const std::string &value) {
            int len = (int)value.size();
            if (len == 0) {
                return false;
            }

            const char *p = value.c_str();
            for (int i = 0; i < len; i++) {
                if (isdigit(p[i]) == 0) {
                    return false;
                }
            }

            return true;
        }

        bool is_float_value(const std::string &value) {
            int len = (int)value.size();
            if (len == 0) {
                return false;
            }

            int point_cnt = 0;
            const char *p = value.c_str();
            for (int i = 0; i < len; i++) {
                if (p[i] == '.') {
                    if (i == 0 || point_cnt > 0) {
                        return false;
                    }
                    point_cnt++;
                }
                if (isdigit(p[i]) == 0) {
                    return false;
                }
            }

            return true;
        }

        bool parse_option_name(bool is_short_option, const char *arg, std::string &name, std::string &value) {
            int len = (int)strlen(arg);
            int beg = is_short_option ? 1 : 2;
            for (int i = beg; i < len; i++) {
                if (arg[i] == '=') {
                    if (is_short_option || i + 1 == len) {
                        return false;
                    }
                    value.assign(arg + i + 1);
                    break;
                }
                name.append(1, arg[i]);
            }

            return !name.empty();
        }

        int get_system_env(const char *name, std::string &value) {
            const char *tmp = getenv(name);
            if (tmp == NULL) {
                return -1;
            }
            value.assign(tmp);
            return (int)value.size();
        }
    }

    command::command()
      : parent_cmd_(NULL),
        action_cb_(NULL) {
    }

    command::~command() {
        {
            // Free all sub commands
            command_map::iterator beg = public_sub_cmds_.begin();
            for (; beg != public_sub_cmds_.end(); beg++) {
                delete beg->second;
            }
        }

        {
            // Free all sub commands
            command_map::iterator beg = sub_cmds_.begin();
            for (; beg != sub_cmds_.end(); beg++) {
                delete beg->second;
            }
        }
        
        // Free all options
        for (int i = 0; i < (int)options_.size(); i++) {
            delete options_[i];
        }
    }

    void command::add_sub_cmd(command *sub) {
        command_map::iterator it = sub_cmds_.find(sub->name_);
        if (it != sub_cmds_.end()) {
            delete it->second;
            sub_cmds_.erase(it);
        }

        sub_cmds_[sub->name_] = sub;

        sub->parent_cmd_ = this;
    }

    void command::add_public_sub_cmd(command *gsub) {
        command_map::iterator it = public_sub_cmds_.find(gsub->name_);
        if (it != public_sub_cmds_.end()) {
            delete it->second;
            public_sub_cmds_.erase(it);
        }
        public_sub_cmds_[gsub->name_] = gsub;
    }

    void command::get_usage(std::string &des) const {
        if (!desc_.empty()) {
            des.append("\n").append(desc_).append("\n");
        }

        command_map sub_cmds = sub_cmds_;
        __get_public_sub_cmds(sub_cmds);
        for (command_map::const_iterator beg = sub_cmds.begin(); beg != sub_cmds.end(); beg++) {
            if (beg->second == this) {
                sub_cmds.erase(beg);
                break;
            }
        }

        std::string path = __get_cmd_path();
        des.append("\nUsage:\n  ").append(path);
        if (!sub_cmds.empty()) {
            des.append(" [COMMAND]");
        }
        if (!options_.empty()) {
            des.append(" [OPTIONS]");
        }
        des.append("\n");

        if (!sub_cmds.empty()) {
            std::string sub_cmds_desc;
            sub_cmds_desc.append("COMMANDS: \n");
            for (command_map::const_iterator beg = sub_cmds.begin(); beg != sub_cmds.end(); beg++) {
                int space_len = (int)(32 - 4 - beg->second->name_.size());
                sub_cmds_desc
                    .append("    ")
                    .append(beg->second->name_)
                    .append(space_len, ' ').append(beg->second->desc_)
                    .append("\n");
            }
            des.append("\n").append(sub_cmds_desc);
        }

        if (!options_.empty()) {
            std::string options_desc;
            options_desc.append("OPTIONS: \n");
            for (option_vector::const_iterator beg = options_.begin(); beg != options_.end(); beg++) {
                option *opt = (option*)(*beg);

                int space_len = 32 - 4;
                options_desc.append("    ");

                if (!opt->short_name_.empty()) {
                    options_desc.append("-").append(opt->short_name_);
                    space_len -= (1 + (int)opt->short_name_.size());
                }
                if (!opt->long_name_.empty()) {
                    if (!opt->short_name_.empty()) {
                        options_desc.append(", ");
                        space_len -= 2;
                    }
                    options_desc.append("--").append(opt->long_name_);
                    space_len -= (2 + (int)opt->long_name_.size());
                }
                options_desc.append(space_len, ' ');
                
                if (opt->required_) {
                    options_desc.append("[Required] ");
                } else {
                    options_desc.append("[Optional] ");
                }
                options_desc.append(opt->desc_).append("\n");
            }
            des.append("\n").append(options_desc);
        }
    }

    void command::print_usage() const {
        std::string usage;
        get_usage(usage);
        printf("%s", usage.c_str());
    }

    int command::run(int argc, const char **argv) {
        if (argc <= 0) {
            __set_error(this, "no arguments");
            return -1;
        }
        return __run_cmd(argv, argc, 1);
    }

    option* command::__create_option(internal::option_type ot, 
                                     const std::string& long_name, 
                                     const std::string& short_name) {
        if (long_name.empty() && short_name.empty()) {
            return nullptr;
        }
        if (__find_option(long_name, short_name)) {
            return nullptr;
        }

        option *opt =  new option(ot, long_name, short_name);
        options_.push_back(opt);

        return opt;
    }

    option* command::__find_option(const std::string& long_name,
                                   const std::string& short_name) const {
        for (int i = 0; i < (int)options_.size(); i++) {
            if (!long_name.empty()) {
                if (options_[i]->long_name_ == long_name) {
                    return options_[i];
                }
            }
            if (!short_name.empty()) {
                if (options_[i]->short_name_ == short_name) {
                    return options_[i];
                }
            }
        }
        return nullptr;
    }

    int command::__run_cmd(const char **argv, int argc, int arg_idx) {
        // If no more args, the command should be handled.
        if (arg_idx >= argc) {
            return __handle_cmd();
        }

        // The next arg is command
        if (internal::is_command_arg(argv[arg_idx])) {
            command *sub = NULL;
            std::string name(argv[arg_idx]);
            command_map::iterator it = sub_cmds_.find(name);
            if (it != sub_cmds_.end()) {
                sub = it->second;
            }

            if (sub == NULL) {
                sub = __get_public_sub_cmd(name);
                if (!sub) {
                    __set_error(this, "no found command: %s\n", name.c_str());
                    return -1;
                }
                sub->parent_cmd_ = this;
            }

            return sub->__run_cmd(argv, argc, arg_idx + 1);
        } else { 
            // Try to setup options from system env.
            __setup_options_from_env();

            // Try to setup options from args
            if (!__setup_options_from_args(argv + arg_idx, argc - arg_idx)) {
                return -1;
            }

            // handle command
            return __handle_cmd();
        }

        return -1;
    }

    int command::__handle_cmd() {
        for (option_vector::iterator beg = options_.begin(); beg != options_.end(); beg++) {
            option *opt = (option*)(*beg);
            if (!opt->found_value_ && opt->required_) {
                if (!opt->long_name_.empty()) {
                    __set_error(this, "option --%s required\n", (*beg)->long_name_.c_str());
                } else {
                    __set_error(this, "option -%s required\n", (*beg)->short_name_.c_str());
                }
                return -1;
            }
        }

        if (action_cb_) {
            int ret = action_cb_(this);
            if (ret != 0) {
                __set_error(this, "process command %s failed\n", this->name_.c_str());
            }
            return ret;
        }

        print_usage();

        return 0;
    }

    void command::__setup_options_from_env() {
        for (int i = 0; i < (int)options_.size(); i++) {
            option *opt = options_[i];
            if (!opt->env_.empty()) {
                std::string value;
                if (internal::get_system_env(opt->env_.c_str(), value) > 0) {
                    __setup_option(opt, value);
                }
            }
        }
    }

    bool command::__setup_options_from_args(const char **argv, int argc) {
        for (int i = 0; i < argc; i++) {
            std::string name;
            std::string value;
            bool is_short_option = false;
            if (internal::is_short_option_arg(argv[i])) {
                is_short_option = true;
            } else if (!internal::is_long_option_arg(argv[i])) {
                __set_error(this, "invalid option: %s\n", argv[i]);
                return false;
            }

            if (!internal::parse_option_name(is_short_option, argv[i], name, value)) {
                __set_error(this, "invalid option: %s\n", argv[i]);
                return false;
            }

            if (is_short_option) {
                for (int j = 0; j < (int)name.size(); j++) {
                    if (j == (int)name.size() - 1) {
                        if (i + 1 < argc && !internal::is_option_arg(argv[i + 1])) {
                            value = argv[i + 1];
                            i++;
                        }
                    }
                    if (!__setup_option(name.substr(j, 1), value)) {
                        __set_error(this, "invalid option: %s\n", argv[i]);
                        return false;
                    }
                }
            } else {
                if (value.empty() && i + 1 < argc) {
                    if (!internal::is_option_arg(argv[i + 1])) {
                        value = argv[i + 1];
                        i++;
                    }
                }
                if (!__setup_option(name, value)) {
                    __set_error(this, "invalid option: %s\n", argv[i]);
                    return false;
                }
            }
        }

        return true;
    }

    bool command::__setup_option(const std::string &name, const std::string &value) {
        option *opt = __find_option(name, name);
        if (!opt) {
            return false;
        }
        return __setup_option(opt, value);
    }
    
    bool command::__setup_option(option *opt, const std::string &value) {
        if (opt->type_ == internal::OP_TYPE_BOOL) {
            if (value.empty() || value == "true" || value == "TRUE") {
                opt->__set(true);                
            } else {
                opt->__set(false);
            }
        } else if (opt->type_ == internal::OP_TYPE_INT) {
            if (!internal::is_int_value(value)) {
                return false;
            }
            opt->__set(atoi(value.c_str()));
        } else if (opt->type_ == internal::OP_TYPE_FLOAT) {
            if (!internal::is_float_value(value)) {
                return false;
            }
            opt->__set(atof(value.c_str()));
        } else if (opt->type_ == internal::OP_TYPE_STRING) {
            if (value.empty()) {
                return false;
            }
            opt->__set(value);
        } else {
            return false;
        }

        return true;
    }

    command* command::__get_public_sub_cmd(const std::string &name) {
        command_map::iterator it = public_sub_cmds_.find(name);
        if (it != public_sub_cmds_.end()) {
            return it->second;
        }

        if (parent_cmd_) {
            return parent_cmd_->__get_public_sub_cmd(name);
        }

        return NULL;
    }

    void command::__get_public_sub_cmds(command_map &cmds) const {
        command_map::const_iterator beg;
        for (beg = public_sub_cmds_.begin(); beg != public_sub_cmds_.end(); beg++) {
            if (cmds.find(beg->first) == cmds.end()) {
                cmds[beg->first] = beg->second;
            }
        }

        if (parent_cmd_) {
            parent_cmd_->__get_public_sub_cmds(cmds);
        }
    }

    std::string command::__get_cmd_path() const {
        if (parent_cmd_) {
            return parent_cmd_->__get_cmd_path() + " " + name_;
        }

#if defined(WIN32)
        size_t pos = name_.rfind('\\');
#else
        size_t pos = name_.rfind('/');
#endif
        if (pos == std::string::npos) {
            return name_;
        }

        return name_.substr(pos+1);
    }

    void command::__set_error(command *cmd, const char *format, ...) {
        va_list args;
        va_start(args, format);

        char msg[1024] = {0};
#if defined(WIN32)
        vsprintf_s(msg, sizeof(msg)-1, format, args);
#else
        vsnprintf(msg, sizeof(msg)-1, format, args);
#endif
        command *root = cmd;
        while (root->parent_cmd_) {
            root = root->parent_cmd_;
        }
        root->err_ = msg;

        va_end(args);
    }
}
