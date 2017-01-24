#include "Constants.h"
#include "Server.h"

int main(int argc, char **argv)
{
	Server serv(3100);
	serv.start();
	return 0;
}
