#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort

#include "imgui_dlg.h"


int main(int, char**)
{
	ImGuiDlg dlg;

	dlg.exec();
	
	return 0;
}
