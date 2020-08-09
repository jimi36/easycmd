#include <easycmd/command.h>

void error(const std::string &msg)
{
	printf("%s", msg.c_str());
}

int func(const easycmd::command *cmd)
{
	std::string usgae;
	//cmd->get_parent_command()->get_help(usgae);
	cmd->get_help(usgae);
	printf("%s", usgae.c_str());

	return 0;
}

int main(int argc, const char **argv)
{
	easycmd::command *sub = new easycmd::command();
	sub->with_name("start")->with_desc("Start server");
	sub->create_option_bool("with_iocp")
		->with_env("WITH_IOCP")
		->with_desc("Using iocp on windows")
		->with_default(false);
	sub->create_option_int("port")
		->with_env("LISTEN_PORT")
		->with_desc("Server listen port");
	sub->create_option_string("ip")
		->with_env("PATH")
		->with_desc("Server listen ip")
		->with_default("127.0.0.1");

	easycmd::command *comm_sub = new easycmd::command();
	comm_sub->with_name("help")->with_desc("Command help")->with_action(func);

	easycmd::command cmd;
	cmd.with_name(argv[0])->with_fault(error);

	cmd.add_sub_cmd(sub);
	cmd.add_comm_sub_cmd(comm_sub);

	int ret = cmd.run(argc, argv);

	return 0;
}