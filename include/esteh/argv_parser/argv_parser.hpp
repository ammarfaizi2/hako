
#ifndef ESTEH_ARGV_PARSER_ARGV_PARSER_HPP
#define ESTEH_ARGV_PARSER_ARGV_PARSER_HPP

#include <esteh/argv_parser/opt_struct.hpp>

class argv_parser 
{
public:
	argv_parser(int argc, char **argv, opt_struct ***opts);
	bool opt1(char *opt);
	bool opt2(char *opt);
	void run(char **filename);
	bool is_error();
	char *get_error();
private:
	void parse();
	int argc;
	bool skip = false;	
	char **argv;
	char *error = nullptr;
	opt_struct ***opts;
};

#endif