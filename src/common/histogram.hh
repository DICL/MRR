#ifndef __HISTOGRAM__
#define __HISTOGRAM__

#include <iostream>
#include <common/ecfs.hh>

#define MAX_UINT 4294967295

using namespace std;

class histogram
{
    private:
        int numserver; // number of server
        int numbin; // number of bin -> number of histogram bin
        // int digit; // number of digits to represent the problem space
        double* querycount; // the data access count to each
        unsigned* boundaries; // the index of end point of each node
        
    public:
        histogram(); // constructs an uninitialized object
        histogram (int numserver, int numbin);   // number of bin and number of digits
        ~histogram();
        
        void initialize(); // partition the problem space equally to each bin
        void init_count(); // initialize the all query counts to zero
        unsigned get_boundary (int index);
        void set_boundary (int index, unsigned boundary);
        double get_count (int index);
        void set_count (int index, double count);
        int get_index (unsigned query);   // return the dedicated node index of query
        int count_query (unsigned query);
        void updateboundary();
        
        void set_numbin (int num);
        int get_numbin();
        
        void set_numserver (int num);
        int get_numserver();
};

#endif
