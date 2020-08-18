#include <easycmd/command.h>

int help(const easycmd::command *cmd)
{
	printf("help\n");
	return 0;
}

int start(const easycmd::command *cmd)
{
	printf("start\n");
	return 0;
}

int main(int argc, const char **argv)
{
	easycmd::command *sub = new easycmd::command();
	sub->with_name("start")->with_desc("Start server")->with_action(start);
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
	comm_sub->with_name("help")->with_desc("Command help");

	easycmd::command cmd;
	cmd.with_name(argv[0])->with_desc("Test easy command");

	cmd.add_sub_cmd(sub);
	cmd.add_comm_sub_cmd(comm_sub);

	int ret = cmd.run(argc, argv);

	return ret;
}
