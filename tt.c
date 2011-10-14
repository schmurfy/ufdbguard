#include "db.h"

/* Override any gcc2 internal prototype to avoid an error.  */
#ifdef __cplusplus
extern "C"
#endif
/* We use char because int might match the return type of a gcc2
   builtin and then its argument prototype would still apply.  */
int
main ()
{
  int major, minor, patch;
db_version ( &major, &minor, &patch );
  ;
  return 0;
}

