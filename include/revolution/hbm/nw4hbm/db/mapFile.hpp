#ifndef NW4R_DB_MAP_FILE_H
#define NW4R_DB_MAP_FILE_H

/*******************************************************************************
 * headers
 */

#include "revolution/types.h"

#include "revolution/os/OSLink.h" // OSModuleInfo

/*******************************************************************************
 * types
 */

namespace nw4hbm { namespace db
{
	// [SPQE7T]/ISpyD.elf:.debug_info::0x39b5d8
	struct MapFile
	{
		byte_t			*mapBuf;		// size 0x04, offset 0x00
		OSModuleInfo	*moduleInfo;	// size 0x04, offset 0x04
		s32				fileEntry;		// size 0x04, offset 0x08
		MapFile			*next;			// size 0x04, offset 0x0c
	}; // size 0x10
}} // namespace nw4hbm::db

/*******************************************************************************
 * functions
 */

namespace nw4hbm { namespace db
{
	bool MapFile_Exists();
	bool MapFile_QuerySymbol(u32 address, u8 *strBuf, u32 strBufSize);
}} // namespace nw4hbm::db

#endif // NW4R_DB_MAP_FILE_H
