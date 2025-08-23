#include <cstdint>
#include "proto/file_common.pb.h"

namespace ztofs
{
namespace server
{

struct FileAttr 
{
    uint64_t size{0};
    uint64_t atime{0}; // Access time
    uint64_t mtime{0}; // Modification time
    uint64_t ctime{0}; // Change time
    uint32_t mode{0};  // File mode (permissions)
    uint32_t uid{0};   // User ID of owner
    uint32_t gid{0};   // Group ID of owner

    void FromPB(const FileAttrPB& pb) {
        size = pb.size();
        atime = pb.atime();
        mtime = pb.mtime();
        ctime = pb.ctime();
        mode = pb.mode();
        uid = pb.uid();
        gid = pb.gid();
    }
    
    void ToPB(FileAttrPB* pb) const {
        pb->set_size(size);
        pb->set_atime(atime);
        pb->set_mtime(mtime);
        pb->set_ctime(ctime);
        pb->set_mode(mode);
        pb->set_uid(uid);
        pb->set_gid(gid);
    }
};

}
}