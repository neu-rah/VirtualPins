#include "Arduino.h"
#include "virtual_pins.h"

char vpins_data[VPINS_SZ];

bool portBranch::vpins_running=false;

void vpins_init() {
	if (portBranch::running()) return;
	for(char n=0;n<VPINS_SZ;n++)
		vpins_data[n]=0;
	portBranch::vpins_running=true;
}

#ifdef VPINS_OPTIMIZE_SPEED
	//sory this should go to RAM :(
	char port_to_branch[] = {
		NOT_A_BRANCH,//0
		NOT_A_BRANCH,//1
		NOT_A_BRANCH,//2
		NOT_A_BRANCH,//3
		NOT_A_BRANCH,//4
		NOT_A_BRANCH,//5
		NOT_A_BRANCH,//6
		NOT_A_BRANCH,//7
		NOT_A_BRANCH,//8
		NOT_A_BRANCH,//9
		NOT_A_BRANCH,//10
		NOT_A_BRANCH,//11
		NOT_A_BRANCH,//12
		NOT_A_BRANCH,//13
		NOT_A_BRANCH,//14
		NOT_A_BRANCH,//15
		NOT_A_BRANCH,//16
	};
#endif

portBranch* tree[branchLimit]={NOBRANCH,NOBRANCH,NOBRANCH,NOBRANCH};

//portBranch::portBranch(char sz):size(sz),localPort(port),active(false) {}

portBranch::portBranch(char port, char sz):size(sz),localPort(port),active(false) {
	//find a free branch
	for(int b=0;b<branchLimit;b++)
		if (tree[b]==NOBRANCH) {
			tree[b]=this;
			index=b;
			#ifdef VPINS_OPTIMIZE_SPEED
				for(int p=localPort+size-1;p>=localPort;p--)
					port_to_branch[p]=b;
			#endif
			active=true;
			break;
		}
}

portBranch::~portBranch() {
	#ifdef VPINS_OPTIMIZE_SPEED
		for(int p=localPort+size-1;p>=localPort;p--)
			port_to_branch[localPort]=NOT_A_BRANCH;
	#endif
	tree[index]=NOBRANCH;
	active=false;
}

int portBranch::pin(int p) {return NUM_DIGITAL_PINS+((localPort-VPA)<<3)+p;}

char portBranch::getBranchId(char port) {
	#ifdef VPINS_OPTIMIZE_SPEED
		return port_to_branch[port];
	#else
		for(int b=0;b<branchLimit;b++)
			if (tree[b] && tree[b]->hasPort) return b;
		return NOT_A_BRANCH;
	#endif
}

portBranch& portBranch::getBranch(char port) {return *tree[portBranch::getBranchId(port)];}

void portBranch::mode() {}//default branch type does nothing
void portBranch::in() {}//default branch type does nothing
void portBranch::out() {}//default branch type does nothing
void portBranch::io() {}//default branch type does nothing

//glue functions calling C++ class methods from C --------------------------
inline void _mode(char port) {
	if (!portBranch::running()) return;
	char branchId=portBranch::getBranchId(port);
	//this check can be removed if you know what are you doing...
	if (branchId==NOT_A_BRANCH || branchId<0 || branchId>=branchLimit) return;
	tree[branchId]->mode();
}

inline void _in(char port) {
	Serial.println(port);
	if (!portBranch::running()) return;
	char branchId=portBranch::getBranchId(port);
	//this check can be removed if you know what are you doing...
	if (branchId==NOT_A_BRANCH || branchId<0 || branchId>=branchLimit) return;
	tree[branchId]->in();
}

inline void _out(char port) {
	if (!portBranch::running()) return;
	char branchId=portBranch::getBranchId(port);
	//this check can be removed if you know what are you doing...
	if (branchId==NOT_A_BRANCH || branchId<0 || branchId>=branchLimit) return;
	tree[branchId]->out();
}

inline void _io(char port) {
	if (!portBranch::running()) return;
	char branchId=portBranch::getBranchId(port);
	//this check can be removed if you know what are you doing...
	if (branchId==NOT_A_BRANCH || branchId<0 || branchId>=branchLimit) return;
	tree[branchId]->io();
}

void vpins_mode(char port) {
	_mode(port);
}
void vpins_in(char port) {_in(port);}
void vpins_out(char port) {_out(port);}
void vpins_io(char port) {_io(port);}
