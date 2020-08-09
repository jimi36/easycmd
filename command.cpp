/*
 * MIT License
 * 
 * Copyright (c) 2020 jimi36
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

namespace easycmd {

	namespace internal
	{
		bool __is_arg_command(const char *arg)
		{
			int len = strlen(arg);
			for (int i = 0; i < len; i++)
			{
				if (isalpha(arg[i++]) == 0)
					return false;
			}
			return true;
		}

		bool __is_arg_option(const char *arg)
		{
			int len = strlen(arg);
			if (len < 3)
				return false;

			if (strncmp(arg, "--", 2) != 0 || isalpha(arg[2]) == 0)
				return false;

			return true;
		}

		bool __is_int_value(const std::string &value)
		{
			int len = value.size();
			if (len == 0)
				return false;

			const char *p = value.c_str();
			for (int i = 0; i < len; i++)
			{
				if (isdigit(p[i]) == 0)
					return false;
			}

			return true;
		}

		bool __is_float_value(const std::string &value)
		{
			int len = value.size();
			if (len == 0)
				return false;

			int point_cnt = 0;
			const char *p = value.c_str();
			for (int i = 0; i < len; i++)
			{
				if (p[i] == '.')
				{
					if (i == 0 || point_cnt > 0)
						return false;
					point_cnt++;
				}
				if (isdigit(p[i]) == 0)
					return false;
			}

			return true;
		}

		bool __parse_option_name(const char *arg, std::string &name)
		{
			if (!__is_arg_option(arg))
				return false;

			int len = strlen(arg);
			for (int i = 2; i < len; i++)
			{
				if (arg[i] == '=')
					break;

				name.append(1, arg[i]);
			}

			return !name.empty();
		}

		int __parse_option_value(const char *arg1, const char *arg2, std::string &value)
		{
			bool found = false;
			int len = strlen(arg1);
			for (int i= 2; i < len; i++)
			{
				if (!found)
				{
					if (arg1[i] == '=')
						found = true;
					continue;
				}
				value.append(1, arg1[i]);
			}

			if (found && !value.empty())
				return 0;

			if (!found && arg2 != NULL)
			{
				if (__is_arg_option(arg2))
					return -1;

				value.assign(arg2);

				return 1;
			}

			return -1;
		}

	}

	command::command():
		action_cb_(NULL),
		fault_cb_(NULL),
		parent_cmd_(NULL)
	{
	}

	command::~command()
	{
		{
			// Free all sub commands
			command_map::iterator beg;
			for (beg = comm_sub_cmds_.begin(); beg != comm_sub_cmds_.end(); beg++)
			{
				delete beg->second;
			}
		}

		{
			// Free all sub commands
			command_map::iterator beg;
			for (beg = sub_cmds_.begin(); beg != sub_cmds_.end(); beg++)
			{
				delete beg->second;
			}
		}
		
		{
			// Free all options
			option_map::iterator beg;
			for (beg = options_.begin(); beg != options_.end(); beg++)
			{
				delete beg->second;
			}
		}
	}

	void command::add_sub_cmd(command *sub)
	{
		command_map::iterator it = sub_cmds_.find(sub->name_);
		if (it != sub_cmds_.end())
		{
			delete it->second;
			sub_cmds_.erase(it);
		}
		sub_cmds_[sub->name_] = sub;

		sub->parent_cmd_ = this;
	}

	void command::add_comm_sub_cmd(command *sub)
	{
		command_map::iterator it = comm_sub_cmds_.find(sub->name_);
		if (it != comm_sub_cmds_.end())
		{
			delete it->second;
			comm_sub_cmds_.erase(it);
		}
		comm_sub_cmds_[sub->name_] = sub;

		sub->parent_cmd_ = this;
	}

	option* command::create_option_bool(const std::string &name)
	{
		option *opt = new option(internal::OP_TYPE_BOOL, name);

		__append_option(opt);
	
		return opt;
	}

	option* command::create_option_int(const std::string &name) 
	{
		option *opt = new option(internal::OP_TYPE_INT, name);

		__append_option(opt);

		return opt;
	}

	option* command::create_option_float(const std::string &name) 
	{
		option *opt = new option(internal::OP_TYPE_FLOAT, name);

		__append_option(opt);

		return opt;
	}

	option* command::create_option_string(const std::string &name) 
	{
		option *opt = new option(internal::OP_TYPE_STRING, name);

		__append_option(opt);

		return opt;
	}

	void command::get_help(std::string &des) const
	{
		if (!desc_.empty())
			des.append("\n").append(desc_).append("\n");

		command_map sub_cmds = sub_cmds_;
		__load_all_comm_sub_commands(sub_cmds);
		for (command_map::const_iterator beg = sub_cmds.begin(); beg != sub_cmds.end(); beg++)
		{
			if (beg->second == this)
			{
				sub_cmds.erase(beg);
				break;
			}
		}

		std::string path = __get_command_path();
		des.append("\nUsage: ").append(path);
		if (!sub_cmds.empty())
			des.append(" [COMMAND]");
		if (!options_.empty())
			des.append(" [OPTIONS]");
		des.append("\n");

		if (!sub_cmds.empty())
		{
			std::string sub_cmds_desc;
			sub_cmds_desc.append("  ").append("COMMANDS: \n");
			for (command_map::const_iterator beg = sub_cmds.begin(); beg != sub_cmds.end(); beg++)
			{
				sub_cmds_desc
					.append("    ")
					.append(beg->second->name_)
					.append("\t\t").append(beg->second->desc_)
					.append("\n");
			}
			des.append("\n").append(sub_cmds_desc).append("\n");
		}

		if (!options_.empty())
		{
			std::string options_desc;
			options_desc.append("  ").append("OPTIONS: \n");
			for (option_map::const_iterator beg = options_.begin(); beg != options_.end(); beg++)
			{
				options_desc
					.append("    --").append(beg->first)
					.append("\t\t").append(beg->second->desc_)
					.append("\n");
			}
			des.append("\n").append(options_desc);
		}
	}

	const option* command::get_option(const std::string &name) const
	{
		option_map::const_iterator it = options_.find(name);
		if (it == options_.end())
			return NULL;
		return it->second;
	}

	int command::run(int argc, const char **argv)
	{
		if (argc < 0)
			return -1;

		if (argc == 1)
		{
			if (action_cb_)
				return action_cb_(this);
			else
				return 0;
		}

		std::string err;
		int ret = __run_command(argv, argc, 1, err);

		if (!err.empty())
		{
			err.append("\n");
			__occur_fault(err);
		}

		return ret;
	}

	void command::__append_option(option *opt)
	{
		option_map::iterator it = options_.find(opt->name_);
		if (it != options_.end())
		{
			delete it->second;
			options_.erase(it);
		}

		options_[opt->name_] = opt;
	}

	int command::__run_command(const char **argv, int argc, int idx, std::string &err)
	{
		if (idx < argc && internal::__is_arg_command(argv[idx]))
		{
			command *sub = NULL;
			std::string name(argv[idx]);
			command_map::iterator it = sub_cmds_.find(name);
			if (it != sub_cmds_.end())
				sub = it->second;

			if (sub == NULL)
			{
				sub = __get_comm_sub_commands(name);
				if (!sub)
				{
					err.append("no found command: ").append(name);
					return -1;
				}

				sub->parent_cmd_ = this;
			}

			return sub->__run_command(argv, argc, idx + 1, err);
		}
		else
		{
			__load_options_from_env();

			if (!__load_options_from_args(argv + idx, argc - idx, err))
				return -1;

			option_map::iterator opt_beg = options_.begin();
			while (opt_beg != options_.end())
			{
				if (!opt_beg->second->found_value_)
				{
					err.append("required option: --").append(opt_beg->second->name_);
					return false;
				}

				opt_beg++;
			}

			if (action_cb_ != NULL)
				return action_cb_(this);

			return 0;
		}

		return -1;
	}

	void command::__load_options_from_env()
	{
		char value[1024];
		option_map::iterator beg = options_.begin();
		while (beg != options_.end())
		{
			option *opt = (beg++)->second;
			if (!opt->env_.empty())
			{
				size_t len = 0;
				memset(value, 0, sizeof(value));
				auto e = getenv_s(&len, value, sizeof(value), opt->env_.c_str());
				if (len > 0)
				{
					if (opt->type_ == internal::OP_TYPE_BOOL)
					{
						if (strncmp(value, "true", 4) == 0 ||
							strncmp(value, "TRUE", 4) == 0)
							opt->__set(true);
						else
							opt->__set(false);
					}
					else if (opt->type_ == internal::OP_TYPE_INT)
					{
						if (internal::__is_int_value(value))
							opt->__set(atoi(value));
					}
					else if (opt->type_ == internal::OP_TYPE_FLOAT)
					{
						if (internal::__is_float_value(value))
							opt->__set(atof(value));
					}
					else if (opt->type_ == internal::OP_TYPE_STRING)
					{
						opt->__set(value);
					}
				}
			}
		}
	}

	bool command::__load_options_from_args(const char **argv, int argc, std::string &err)
	{
		for (int i = 0; i < argc; i++)
		{
			std::string name, value;
			if (!internal::__parse_option_name(argv[i], name))
			{
				err.append("invalid option: ").append(argv[i]);
				return false;
			}

			option_map::iterator it = options_.find(name);
			if (it == options_.end())
			{
				err.append("no found option: ").append(argv[i]);
				return false;
			}

			if (it->second->type_ == internal::OP_TYPE_BOOL)
			{
				internal::__parse_option_value(argv[i], argv[i + 1], value);
				if (value.empty())
					it->second->__set(true);
				else if (value == "true" || value == "TRUE")
					it->second->__set(true);
				else
					it->second->__set(false);
			}
			else if (it->second->type_ == internal::OP_TYPE_INT)
			{
				int ret = -1;
				if (argc >= i + 1)
					ret = internal::__parse_option_value(argv[i], argv[i + 1], value);
				else
					ret = internal::__parse_option_value(argv[i], NULL, value);

				if (!internal::__is_int_value(value))
				{
					err.append("invalid option: ").append(argv[i]);
					return false;
				}

				it->second->__set(atoi(value.c_str()));

				i += ret;
			}
			else if (it->second->type_ == internal::OP_TYPE_FLOAT)
			{
				int ret = -1;
				if (argc >= i + 1)
					ret = internal::__parse_option_value(argv[i], argv[i + 1], value);
				else
					ret = internal::__parse_option_value(argv[i], NULL, value);

				if (!internal::__is_float_value(value))
				{
					err.append("invalid option: ").append(argv[i]);
					return false;
				}

				it->second->__set(atof(value.c_str()));

				i += ret;
			}
			else if (it->second->type_ == internal::OP_TYPE_STRING)
			{
				int ret = -1;
				if (argc >= i + 1)
					ret = internal::__parse_option_value(argv[i], argv[i + 1], value);
				else
					ret = internal::__parse_option_value(argv[i], NULL, value);

				it->second->__set(value);

				i += ret;
			}
			else
			{
				err.append("invalid option type at: ").append(argv[i]);
				return false;
			}
		}

		return true;
	}

	command* command::__get_comm_sub_commands(const std::string &name)
	{
		command_map::iterator it = comm_sub_cmds_.find(name);
		if (it != comm_sub_cmds_.end())
			return it->second;

		if (parent_cmd_)
			return parent_cmd_->__get_comm_sub_commands(name);

		return NULL;
	}

	void command::__load_all_comm_sub_commands(command_map &cmds) const
	{
		for (command_map::const_iterator beg = comm_sub_cmds_.begin(); beg != comm_sub_cmds_.end(); beg++)
		{
			if (cmds.find(beg->first) == cmds.end())
				cmds[beg->first] = beg->second;
		}

		if (parent_cmd_)
			parent_cmd_->__load_all_comm_sub_commands(cmds);
	}

	std::string command::__get_command_path() const
	{
		if (parent_cmd_)
			return parent_cmd_->__get_command_path() + " " + name_;

#ifdef WIN32
		size_t pos = name_.rfind('\\');
#else
		size_t pos = name_.rfind('/');
#endif
		if (pos == std::string::npos)
			return name_;
		return name_.substr(pos+1);
	}

	void command::__occur_fault(const std::string &msg)
	{
		if (fault_cb_)
			fault_cb_(msg);
		else if (parent_cmd_)
			parent_cmd_->__occur_fault(msg);
	}

}