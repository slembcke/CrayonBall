#include <stdio.h>

#include <ruby.h>

void AS_printException(char *message)
{
	fprintf(stderr, "%s\n", message);
	rb_eval_string("STDERR.puts $!; STDERR.puts $!.backtrace");
	rb_exit(1);
}
