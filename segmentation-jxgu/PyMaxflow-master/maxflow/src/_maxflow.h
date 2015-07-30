#ifndef __PYX_HAVE___maxflow
#define __PYX_HAVE___maxflow

struct PyObject_GraphInt;
struct PyObject_GraphFloat;

/* "_maxflow.pyx":100
 * 
 * 
 * cdef public class GraphInt [object PyObject_GraphInt, type GraphInt]:             # <<<<<<<<<<<<<<
 *     cdef Graph[long, long, long]* thisptr
 *     def __cinit__(self, int est_node_num=0, int est_edge_num=0):
 */
struct PyObject_GraphInt {
  PyObject_HEAD
  Graph<long,long,long>  *thisptr;
};

/* "_maxflow.pyx":496
 * 
 * 
 * cdef public class GraphFloat [object PyObject_GraphFloat, type GraphFloat]:             # <<<<<<<<<<<<<<
 *     cdef Graph[double, double, double]* thisptr
 *     def __cinit__(self, int est_node_num=0, int est_edge_num=0):
 */
struct PyObject_GraphFloat {
  PyObject_HEAD
  Graph<double,double,double>  *thisptr;
};

#ifndef __PYX_HAVE_API___maxflow

#ifndef __PYX_EXTERN_C
  #ifdef __cplusplus
    #define __PYX_EXTERN_C extern "C"
  #else
    #define __PYX_EXTERN_C extern
  #endif
#endif

#ifndef DL_IMPORT
  #define DL_IMPORT(_T) _T
#endif

__PYX_EXTERN_C DL_IMPORT(PyTypeObject) GraphInt;
__PYX_EXTERN_C DL_IMPORT(PyTypeObject) GraphFloat;

#endif /* !__PYX_HAVE_API___maxflow */

#if PY_MAJOR_VERSION < 3
PyMODINIT_FUNC init_maxflow(void);
#else
PyMODINIT_FUNC PyInit__maxflow(void);
#endif

#endif /* !__PYX_HAVE___maxflow */
