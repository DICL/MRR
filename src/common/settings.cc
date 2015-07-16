//
// @author     Vicente Adolfo Bolea Sanchez
// @brief      Settings impl using boost library.
// 
// Includes {{{
#include "settings.hh"
#include <iostream>
#include <vector>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/exceptions.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>
#include <ifaddrs.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h> /* for strncpy */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

#define FINAL_PATH "/eclipse.json"

using std::cout;
using std::endl;
using std::vector;
using std::string;
using namespace boost::property_tree;
//}}}
// class SettingsImpl {{{
class Settings::SettingsImpl {
  protected:
    ptree pt;
    string config_path, given_path;
    bool hardcoded_path;
    bool get_project_path ();

  public:
    SettingsImpl() : hardcoded_path(false) { }
    SettingsImpl(string in) : given_path (in), hardcoded_path (true) { }
    bool load ();

    template <typename T> T get (string&) ;
    string getip () const ;
};
//}}}
// get_project_path {{{
bool Settings::SettingsImpl::get_project_path () 
{
  string home_location   = string(getenv("HOME")) + "/.eclipse.json";
  string system_location = "/etc/eclipse.json";

  if (hardcoded_path) {
    config_path = given_path;                                           // Frist the from constructor

  } else if (access(home_location.c_str(), F_OK) == EXIT_SUCCESS) {     // Then at home
    config_path = home_location;

  } else if (access(system_location.c_str(), F_OK) == EXIT_SUCCESS) {   // Then at /etc
    config_path = system_location;

  } else {                               
#ifdef ECLIPSE_CONF_PATH
    config_path = string(ECLIPSE_CONF_PATH) + FINAL_PATH;               // Then configure one
#else
    return false;
#endif
  }

  return true;
}
// }}}
// load {{{
//
bool Settings::SettingsImpl::load ()
{
  get_project_path();
  json_parser::read_json (config_path, pt);

  return true;
}
// }}}
// getip {{{
string Settings::SettingsImpl::getip () const
{
 int fd;
 struct ifreq ifr;
 char if_ip [INET_ADDRSTRLEN];
 string interface = pt.get<string> ("network.iface");

 fd = socket(AF_INET, SOCK_DGRAM, 0);

 /* I want to get an IPv4 IP address */
 ifr.ifr_addr.sa_family = AF_INET;

 /* I want IP address attached to "eth0" */
 strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ-1);

 ioctl(fd, SIOCGIFADDR, &ifr);
 inet_ntop (AF_INET, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr , if_ip, INET_ADDRSTRLEN);

 close(fd);

  return string (if_ip);
}
// }}}
// Get specializations {{{
template<typename T> T Settings::SettingsImpl::get (string& str) { 
   return pt.get<T> (str.c_str()); 
}

template string Settings::SettingsImpl::get (string& str);
template int    Settings::SettingsImpl::get (string& str);

template<> vector<string> Settings::SettingsImpl::get (string& str) { 
  vector<string> output;
  BOOST_FOREACH(ptree::value_type& v, pt.get_child (str.c_str())) {
    output.push_back (v.second.data());
  }
  return output;
}
//}}}
// Settings method{{{
//
Settings::Settings() { impl = new SettingsImpl(); }
Settings::Settings(string in) { impl = new SettingsImpl(in); }
Settings::~Settings() { delete impl; }

bool Settings::load () { return impl->load (); }
string Settings::getip () const { return impl->getip(); }
 
template<typename T> T Settings::get (string str) const { 
  return impl->get<T>(str); 
}

template int            Settings::get (string) const;
template string         Settings::get (string) const;
template vector<string> Settings::get (string) const;
// }}}
