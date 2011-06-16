/*
    Zeta CL, OpenCL Chess Engine
    Author: Srdja Matovic <srdja@matovic.de>
    Created at: 20-Jan-2011
    Updated at:
    Description: A Chess Engine written in OpenCL, a language suited for GPUs.

    Copyright (C) 2011 Srdja Matovic
    This program is distributed under the GNU General Public License.
    See file COPYING or http://www.gnu.org/licenses/
*/

#if !defined(OPENCL_H_INCLUDED)
#define OPENCL_H_INCLUDED

#include "types.h"

int load_file_to_string(const char *filename, char **result);
void print_debug(char *debug);

extern char *source;
extern size_t sourceSize;
extern Move bestmove;
extern Move *MOVES;
extern Bitboard *BOARDS;
extern long NODECOUNT;
extern long MOVECOUNT;
extern Bitboard *SetMaskBB;
extern Bitboard *ClearMaskBB;
extern Bitboard *AttackTables;
extern Bitboard *AttackTablesTo;
extern Bitboard *PawnAttackTables;
extern Bitboard *OutputBB;
extern Bitboard *avoidWrap;
extern signed int *shift;
extern int *BitTable;

extern int RAttackIndex[64];
extern Bitboard RAttacks[0x19000];
extern Bitboard RMask[64];

extern int BAttackIndex[64];
extern Bitboard BAttacks[0x1480];
extern Bitboard BMask[64];


cl_mem   BoardBuffer;
cl_mem	 BestmoveBuffer;
cl_mem	 MoveBuffer;
cl_mem	 MovecountBuffer;
cl_mem	 NodecountBuffer;
cl_mem	 SetMaskBBBuffer;
cl_mem	 ClearMaskBBBuffer;
cl_mem	 AttackTablesBuffer;
cl_mem	 AttackTablesToBuffer;
cl_mem	 PawnAttackTablesBuffer;
cl_mem	 OutputBBBuffer;
cl_mem	 avoidWrapBuffer;
cl_mem	 shiftBuffer;
cl_mem	 BitTableBuffer;

cl_mem	 RAttackIndexBuffer;
cl_mem	 RAttacksBuffer;
cl_mem	 RMaskBuffer;
cl_mem	 BAttackIndexBuffer;
cl_mem	 BAttacksBuffer;
cl_mem	 BMaskBuffer;

cl_context          context;
cl_device_id        *devices;
cl_command_queue    commandQueue;
cl_program          program;
cl_kernel           kernel;


#endif




