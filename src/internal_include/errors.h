//
// Created by christianb on 18.07.20.
//

#ifndef RTSP_PARSER_ERRORS_H
#define RTSP_PARSER_ERRORS_H
#define ERR_OK          0    /* No error, everything OK. */
#define ERR_MEM        -1    /* Out of memory error.     */
#define ERR_BUF        -2    /* Buffer error.            */
#define ERR_TIMEOUT    -3    /* Timeout.                 */
#define ERR_RTE        -4    /* Routing problem.         */

#define ERR_IS_FATAL(e) ((e) < ERR_RTE)

#define ERR_ABRT       -5    /* Connection aborted.      */
#define ERR_RST        -6    /* Connection reset.        */
#define ERR_CLSD       -7    /* Connection closed.       */
#define ERR_CONN       -8    /* Not connected.           */

#define ERR_VAL        -9    /* Illegal value.           */

#define ERR_ARG        -10   /* Illegal argument.        */

#define ERR_USE        -11   /* Address in use.          */

#define ERR_IF         -12   /* Low-level netif error    */
#define ERR_ISCONN     -13   /* Already connected.       */

#define ERR_INPROGRESS -14   /* Operation in progress    */
#endif //RTSP_PARSER_ERRORS_H
