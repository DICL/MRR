dnl 
dnl USAGE: just call ECLIPSE_FIND_BOOST no arguments
dnl 
dnl AUTHOR: Vicente Adolfo Bolea Sanchez
dnl
AC_DEFUN([ECLIPSE_FIND_BOOST], 
[
# BOOST LIB {{{
have_boost=no
is_supported=yes

OS=`cat /etc/*release | sed -rn 's/.*(Ubuntu|CentOS).*/\1/p; q'`
case $OS in
  Ubuntu) ;;
  CentOS) boost_path=`file /usr/local/include/boost* | cut -d ":" -f 1`
          CPPFLAGS="-I $boost_path"
          ;;
  *)      is_supported=no ;;
esac 

AC_CHECK_HEADERS([boost/foreach.hpp \
                  boost/property_tree/json_parser.hpp \
                  boost/property_tree/exceptions.hpp], [have_boost=yes])

if test "${have_boost}" = "no"; then
  WARN([
-------------------------------------------------
 I cannot find where you have the boost header files...
 OS supported?...: $is_supported
 Our OS guess?...: $is_supported
 booth path......: $booth_path
 
 Re-run configure script in this way:
 \033@<:@31m
   $ CPLUS_INCLUDE_PATH=/path/to/boost ./configure
 \033@<:@0m
-------------------------------------------------\n])
  AC_MSG_ERROR
fi # }}}
])
