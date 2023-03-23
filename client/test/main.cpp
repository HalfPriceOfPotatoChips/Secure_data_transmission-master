#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "RequestCodec.h"
#include "RespondCodec.h"
#include "RequestFactory.h"
#include "RespondFactory.h"
#include "ClientOperation.h"
#include <sys/ipc.h>
#include <sys/shm.h>

#include <fstream>
#include <sstream>


using namespace std;
int usage();

int main()
{
	ClientOperation client("cle_config.json");

	//enum CmdType{NewOrUpdate=1, Check, Revoke, View};
	int nSel;
	while (1)
	{
		nSel = usage();
		switch(nSel)
		{
		case Key::RANDOMNUM +1:
			client.secKeyAgree();
			break;

		case Key::KEYCHECK:
			client.secKeyCheck();
			break;
		case Key::KEYREVOKE:
			client.secKeyRevoke();
			break;
		case 0:
		     exit(0);

		default:
			break;
		}
	}	
}

int usage()
{
	int nSel = -1;
	printf("\n  /*************************************************************/");
	printf("\n  /*************************************************************/");
	printf("\n  /*     1.密钥协商                                            */");
	printf("\n  /*     2.密钥校验                                          */");
	printf("\n  /*     3.密钥注销                                            */");
	printf("\n  /*     4.密钥查看                                           */");
	printf("\n  /*     0.退出系统                                        */");
	printf("\n  /*************************************************************/");
	printf("\n  /*************************************************************/");
	printf("\n\n  选择:");

	scanf("%d", &nSel);
	while (getchar() != '\n');

	return nSel;
}
