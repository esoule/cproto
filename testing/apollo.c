#define	const
/* LINT_PREPRO6
#undef	COMPLETE_SR10_2
#ifdef	__STDC__
#ident	Standard LLIB
#else
#ident	Traditional LLIB
#endif
*/

/* LINT_EXTERN */
#include	<apollo/base.h>

/*		<apollo/a.out.h>	*/
#include	<apollo/aclm.h>
/*		<apollo/aouthdr.h>	*/
/*		<apollo/builtins.h>	*/
/*		<apollo/cache.h>	*/

#include	<apollo/cal.h>

/*		<apollo/ctm.h>		(see below) */
/*		<apollo/dialog.h>	(separate) */
/*		<apollo/dsee.h>		(separate) */

#ifdef	COMPLETE_SR10_2
#include	<apollo/dst.h>
#include	<apollo/ec2.h>
#include	<apollo/errlog.h>
#endif

#include	<apollo/error.h>
#include	<apollo/ev.h>

/*		<apollo/fault.h>	*/
/*		<apollo/filehdr.h>	*/

#ifdef	COMPLETE_SR10_2
#include	<apollo/fontn.h>
#include	<apollo/fpp.h>
#include	<apollo/fpp1.h>
#include	<apollo/gmf.h>
#endif

/*		<apollo/gmr.h>		(missing?) */
/*		<apollo/gpr.h>		(lint can't eat prototype-in!) */
/*		<apollo/gsr.h>		(missing?) */
/*		<apollo/gsr_types.h>	*/

#ifdef	COMPLETE_SR10_2
#include	<apollo/io_traits.h>
#endif
#include	<apollo/ios.h>
#ifdef	COMPLETE_SR10_2
#include	<apollo/ios_dir.h>
#include	<apollo/ios_dir_trait.h>
#include	<apollo/ipc.h>
/*		<apollo/kbd.h>		*/
/*		<apollo/linenum.h>	*/
#include	<apollo/loader.h>
#include	<apollo/mbx.h>
/*		<apollo/mir.h>		*/
#include	<apollo/ms.h>
#include	<apollo/mts.h>
#include	<apollo/mutex.h>
#endif

#include	<apollo/name.h>
/*		<apollo/nlist.h>	*/

#ifdef	COMPLETE_SR10_2
#include	<apollo/org.h>
#include	<apollo/osinfo.h>
#endif

#include	<apollo/pad.h>

#ifdef	COMPLETE_SR10_2
#include	<apollo/pbufs.h>
#include	<apollo/pfm.h>
#include	<apollo/pgm.h>
#include	<apollo/pm.h>
#include	<apollo/ppfm.h>
#include	<apollo/prf.h>
#include	<apollo/proc1.h>
#include	<apollo/proc2.h>
/*		<apollo/reloc.h>	*/
#include	<apollo/rws.h>
/*		<apollo/scndata.h>	*/
/*		<apollo/scnhdr.h>	*/
#include	<apollo/sfcb.h>
#include	<apollo/sio.h>
#include	<apollo/sio_hw.h>
#include	<apollo/sio_trait.h>
#include	<apollo/smdu.h>
#include	<apollo/sri.h>
/*		<apollo/storclass.h>	*/
#include	<apollo/symbol.h>
/*		<apollo/syms.h>		*/
#include	<apollo/task.h>
#include	<apollo/time.h>
#include	<apollo/tml.h>
#include	<apollo/tone.h>
#include	<apollo/tpad.h>
#endif

/* patch:#include	<apollo/trait.h> */
#include	<apollo/tty.h>
#include	<apollo/type_uids.h>

#ifdef	COMPLETE_SR10_2
/*		<apollo/unwind.h>	*/
#include	<apollo/vec.h>
#include	<apollo/vfmt.h>

#include	<apollo/ctm.h>
#endif
