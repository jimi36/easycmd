#include <easycmd/command.h>

int help(const easycmd::command *cmd)
{
	cmd->get_parent_cmd()->print_usage();
	return 0;
}

int start(const easycmd::command *cmd)
{
	printf("ip: %s port:%d enable: %d\n", 
		cmd->get_option("ip")->get_string().c_str(),
		cmd->get_option("p")->get_int(),
		cmd->get_option("e")->get_bool());
	return 0;
}

int main(int argc, const char **argv)
{
	easycmd::command *sub = new easycmd::command();
	sub->with_name("start")->with_desc("Start server")->with_action(start);
	sub->create_option_bool("enable", "e")
		->with_env("ENABLE")
		->with_desc("Using iocp on windows")
		->with_default(false);
	sub->create_option_int("port", "p")
		->with_env("PORT")
		->with_desc("Server listen port")
		->with_default(88);
	sub->create_option_string("ip", "")
		->with_env("IP")
		->with_desc("Server listen ip")
		->with_default("127.0.0.1");

	easycmd::command *comm_sub = new easycmd::command();
	comm_sub->with_name("help")->with_action(help)->with_desc("Command help");

	easycmd::command cmd;
	cmd.with_name(argv[0])->with_desc("Test easy command");

	cmd.add_sub_cmd(sub);
	cmd.add_public_sub_cmd(comm_sub);

	int ret = cmd.run(argc, argv);
	if (ret != 0) {
		printf("%s\n", cmd.get_err().c_str());
	}

	return ret;
}
