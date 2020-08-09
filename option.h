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

#ifndef easycmd_option_h
#define easycmd_option_h

#include <string>

namespace easycmd {

	namespace internal {

		/*********************************************************************************
		 * Option types
		 ********************************************************************************/
		enum option_type
		{
			OP_TYPE_BOOL = 0,
			OP_TYPE_INT,
			OP_TYPE_FLOAT,
			OP_TYPE_STRING
		};
	}

	/*********************************************************************************
	 * Option
	 ********************************************************************************/
	class option
	{
	protected:
		friend class command;

	public:
		/*********************************************************************************
		 * Constructor
		 ********************************************************************************/
		option(internal::option_type ot, const std::string &name): 
			type_(ot),
			name_(name),
			found_value_(false)
		{}

		/*********************************************************************************
		 * Deconstructor
		 ********************************************************************************/
		~option()
		{}

		/*********************************************************************************
		 * Set environmnet
		 ********************************************************************************/
		option* with_env(const std::string &env) { env_ = env; return this; }

		/*********************************************************************************
		 * Set desc
		 ********************************************************************************/
		option* with_desc(const std::string &usage) { desc_ = usage; return this; }

		/*********************************************************************************
		 * Set default value
		 ********************************************************************************/
		option* with_default(int value) { __set(value); return this; }
		option* with_default(bool value) { __set(value); return this; }
		option* with_default(double value) { __set(value); return this; }
		option* with_default(const std::string &value) { __set(value); return this; }

		/*********************************************************************************
		 * Get value
		 ********************************************************************************/
		int get_int() const { return val_.i; }
		bool get_bool() const { return val_.b; }
		double get_float() const { return val_.f; }
		const std::string& get_string() const { return val_s_; }

	private:
		/*********************************************************************************
		 * Set value
		 ********************************************************************************/
		void __set(int value)  { found_value_ = true; val_.i = value; }
		void __set(bool value) { found_value_ = true; val_.b = value; }
		void __set(double value) { found_value_ = true; val_.f = value; }
		void __set(const char *value) { found_value_ = true; val_s_ = value; }
		void __set(const std::string &value) { found_value_ = true; val_s_ = value; }

	private:
		// option type
		int type_;	

		// option name
		std::string name_;
		// option env
		std::string env_;
		// option desc
		std::string desc_;

		// found value status
		// true if has default value
		bool found_value_;

		// values
		union
		{
			int i;
			bool b;
			double f;
		} val_;
		std::string val_s_;
	};

}

#endif