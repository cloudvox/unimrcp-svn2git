#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)

AC_INIT([unimrcp],[0.4.0])

AC_CONFIG_AUX_DIR([build])
AC_CONFIG_MACRO_DIR([build/acmacros])
AC_PREFIX_DEFAULT(/usr/local/unimrcp)

plugindir='${prefix}/plugin'
confdir='${prefix}/conf'
logdir='${prefix}/log'
datadir='${prefix}/data'

AM_INIT_AUTOMAKE(foreign)

#Set default language
AC_LANG_C

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

#skip detection of Fortran
m4_undefine([AC_PROG_F77])
m4_defun([AC_PROG_F77],[])
AC_PROG_LIBTOOL

# get version information
get_version="build/get-version.sh"
version_hdr="build/uni_version.h"
plugin_version_hdr="libs/mrcp-engine/include/mrcp_resource_plugin.h"
UNI_DOTTED_VERSION="`$get_version all $version_hdr UNI`"
UNI_LT_VERSION="-version-info `$get_version libtool $version_hdr UNI`"
PLUGIN_LT_VERSION="-version-info `$get_version libtool $plugin_version_hdr PLUGIN`"

AC_SUBST(UNI_DOTTED_VERSION)
AC_SUBST(UNI_LT_VERSION)
AC_SUBST(PLUGIN_LT_VERSION)

echo "UniMRCP Version: ${UNI_DOTTED_VERSION}"

UNIMRCP_CHECK_APR
UNIMRCP_CHECK_APU

UNIMRCP_CHECK_SOFIA

UNIMRCP_CHECK_SWIFT


AC_SUBST(ac_aux_dir)
AC_SUBST(ac_macro_dir)
AC_SUBST(plugindir)
AC_SUBST(confdir)
AC_SUBST(logdir)

AC_DEFUN([AX_COMPILER_VENDOR],
[
AC_CACHE_CHECK([for _AC_LANG compiler vendor], ax_cv_[]_AC_LANG_ABBREV[]_compiler_vendor,
 [ax_cv_[]_AC_LANG_ABBREV[]_compiler_vendor=unknown
  # note: don't check for gcc first since some other compilers define __GNUC__
  for ventest in intel:__ICC,__ECC,__INTEL_COMPILER ibm:__xlc__,__xlC__,__IBMC__,__IBMCPP__ gnu:__GNUC__ sun:__SUNPRO_C,__SUNPRO_CC hp:__HP_cc,__HP_aCC dec:__DECC,__DECCXX,__DECC_VER,__DECCXX_VER borland:__BORLANDC__,__TURBOC__ comeau:__COMO__ cray:_CRAYC kai:__KCC lcc:__LCC__ metrowerks:__MWERKS__ sgi:__sgi,sgi microsoft:_MSC_VER watcom:__WATCOMC__ portland:__PGI; do
    vencpp="defined("`echo $ventest | cut -d: -f2 | sed 's/,/) || defined(/g'`")"
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM(,[
#if !($vencpp)
      thisisanerror;
#endif
])], [ax_cv_]_AC_LANG_ABBREV[_compiler_vendor=`echo $ventest | cut -d: -f1`; break])
  done
 ])
])

AX_COMPILER_VENDOR


AC_ARG_ENABLE(maintainer-mode,
    [AC_HELP_STRING([--enable-maintainer-mode  ],[turn on debugging and compile time warnings])],
    [enable_maintainer_mode="$enableval"],
    [enable_maintainer_mode="no"])

if test "${enable_maintainer_mode}" != "no"; then
    CFLAGS="$CFLAGS -g"
    if test "x${ax_cv_c_compiler_vendor}"  =  "xgnu" ; then
      CFLAGS="$CFLAGS -Wall -Werror"
    fi
fi

AC_ARG_ENABLE(test-suites,
    [AC_HELP_STRING([--enable-test-suites  ],[build test suites])],
    [enable_test_suites="$enableval"],
    [enable_test_suites="no"])

AM_CONDITIONAL([TEST_SUITES],[test "${enable_test_suites}" != "no"])

AC_ARG_ENABLE(demosynth-plugin,
    [AC_HELP_STRING([--disable-demosynth-plugin  ],[exclude demo synthesizer plugin from build])],
    [enable_demosynth_plugin="$enableval"],
    [enable_demosynth_plugin="yes"])

AM_CONDITIONAL([DEMOSYNTH_PLUGIN],[test "${enable_demosynth_plugin}" = "yes"])

AC_ARG_ENABLE(demorecog-plugin,
    [AC_HELP_STRING([--disable-demorecog-plugin  ],[exclude demo recognizer plugin from build])],
    [enable_demorecog_plugin="$enableval"],
    [enable_demorecog_plugin="yes"])

AM_CONDITIONAL([DEMORECOG_PLUGIN],[test "${enable_demorecog_plugin}" = "yes"])


AC_CONFIG_FILES([
  Makefile
  libs/Makefile
  libs/apr-toolkit/Makefile
  libs/mpf/Makefile
  libs/mrcp/Makefile
  libs/mrcp-signaling/Makefile
  libs/mrcpv2-transport/Makefile
  libs/mrcp-engine/Makefile
  libs/mrcp-server/Makefile
  libs/mrcp-client/Makefile
  libs/uni-rtsp/Makefile
  modules/Makefile
  modules/mrcp-sofiasip/Makefile
  modules/mrcp-unirtsp/Makefile
  plugins/Makefile
  plugins/mrcp-cepstral/Makefile
  plugins/demo-synth/Makefile
  plugins/demo-recog/Makefile
  platforms/Makefile
  platforms/libunimrcp-server/Makefile
  platforms/libunimrcp-client/Makefile
  platforms/unimrcp-server/Makefile
  platforms/unimrcp-client/Makefile
  tests/Makefile
  tests/apttest/Makefile
  tests/mpftest/Makefile
  tests/mrcptest/Makefile
  tests/rtsptest/Makefile
  tests/strtablegen/Makefile
  build/Makefile
  build/pkgconfig/Makefile
  build/pkgconfig/unimrcpclient.pc
  build/pkgconfig/unimrcpserver.pc
  build/pkgconfig/unimrcpplugin.pc
])

AC_OUTPUT
