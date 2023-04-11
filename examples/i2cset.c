#include <stdio.h>
#include <libch341a.h>

#include "common.h"

int main (int argc, char **argv)
{
	struct ch341a_handle *ch341a;

	ch341a = ch341a_open(ch341a_log_cb);

	return 0;
}
