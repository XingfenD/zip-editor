#ifndef DEFS_HPP
#define DEFS_HPP

#include <string>

/* Signatures */
#define LOCAL_FILE_HEADER_SIG 0x04034b50
#define CENTRAL_DIRECTORY_HEADER_SIG 0x02014b50
#define END_OF_CENTRAL_DIRECTORY_SIG 0x06054b50

static const std::string LFH_LENGTH_UNMATCH_KEY("lfh_length_unmatch");

#endif /* DEFS_HPP */
