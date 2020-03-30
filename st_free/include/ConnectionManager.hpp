#include "ST_free.hpp"
#include "StructInformation.hpp"

namespace ST_free {
class ConnectionManager {
 public:
  void get();
  void set();

 private:
  map<StructInformation *, StructInformation *> connectionMap;
};
}  // namespace ST_free
