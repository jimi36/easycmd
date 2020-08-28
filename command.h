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
 
#ifndef options_h
#define options_h

#include <map>
#include <string.h>

#include "option.h"

namespace easycmd {

	/*********************************************************************************
	 * Command
	 ********************************************************************************/
	class command
	{
	public:
		/*********************************************************************************
		 * Command action callback
		 ********************************************************************************/
		typedef int(*action_callback)(const command*);

		/*********************************************************************************
		 * Command error callback
		 ********************************************************************************/
		typedef void(*fault_callback)(const std::string&);

		/*********************************************************************************
		 * Common Types
		 ********************************************************************************/
		typedef std::map<std::string, option*> option_map;
		typedef std::map<std::string, command*> command_map;

	public:
		/*********************************************************************************
		 * Constructor
		 ********************************************************************************/
		command();

		/*********************************************************************************
		 * Deconstructor
		 ********************************************************************************/
		~command();

		/*********************************************************************************
		 * Set command name
		 ********************************************************************************/
		command* with_name(const std::string &name) { name_ = name; return this; }

		/*********************************************************************************
		 * Set command desc
		 ********************************************************************************/
		command* with_desc(const std::string &desc) { desc_ = desc; return this; }

		/*********************************************************************************
		 * Set command action function
		 ********************************************************************************/
		command* with_action(action_callback action) { action_cb_ = action; return this; }

		/*********************************************************************************
		 * Add sub command
		 * If there is already a sub command with the same name, the new sub command 
		 * will replace the old one.
		 ********************************************************************************/
		void add_sub_cmd(command *sub);

		/*********************************************************************************
		 * Add global sub command
		 * All sub commands of the current command will append the global sub command.
		 * If there is already a global sub command with the same name, the new global  
		 * sub command will replace the old one.
		 ********************************************************************************/
		void add_global_sub_cmd(command *gsub);

		/*********************************************************************************
		 * Create option
		 * If there is an old option with the name, the old option will be remove.
		 ********************************************************************************/
		option* create_option_int(const std::string &name);
		option* create_option_bool(const std::string &name);
		option* create_option_float(const std::string &name);
		option* create_option_string(const std::string &name);

		/*********************************************************************************
		 * Get option
		 ********************************************************************************/
		const option* get_option(const std::string &name) const;

		/*********************************************************************************
		 * Run command
		 ********************************************************************************/
		int run(int argc, const char **argv);

		/*********************************************************************************
		 * Get parent command
		 ********************************************************************************/
		const command* get_parent_cmd() const { return parent_cmd_; }

		/*********************************************************************************
		 * Get command help
		 ********************************************************************************/
		void get_help(std::string &des) const;

		/*********************************************************************************
		 * Print command help
		 ********************************************************************************/
		void print_help() const;

	private:
		/*********************************************************************************
		 * Append option
		 ********************************************************************************/
		void __append_option(option *opt);

		/*********************************************************************************
		 * Run command will parse args and call sub command or call action
		 ********************************************************************************/
		int __run_cmd(const char **argv, int argc, int next_arg);

		/*********************************************************************************
		 * Handle command
		 ********************************************************************************/
		int __handle_cmd();

		/*********************************************************************************
		 * Load options from environment
		 ********************************************************************************/
		void __load_options_from_env();

		/*********************************************************************************
		 * Load options from args
		 ********************************************************************************/
		bool __load_options_from_args(const char **argv, int argc);

		/*********************************************************************************
		 * Get common sub command
		 ********************************************************************************/
		command* __get_global_sub_cmd(const std::string &name);

		/*********************************************************************************
		 * Load all global sub commands
		 ********************************************************************************/
		void __load_all_global_sub_cmds(command_map &cmds) const;

		/*********************************************************************************
		 * Get command path
		 ********************************************************************************/
		std::string __get_cmd_path() const;

	private:
		// Command name
		std::string name_;

		// Command desc
		std::string desc_;

		// Command action callback
		action_callback action_cb_;

		// Parent command
		command *parent_cmd_;

		// Sub commands
		command_map sub_cmds_;
		// Global sub commands
		command_map global_sub_cmds_;

		// Command options
		option_map options_;
	};

}

#endif
