#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <list>
#include <chrono>
#include <unistd.h>

using namespace std;

template <typename T>
class BulkReporter
{
public:
    BulkReporter( const T& commands )
    {
        if( commands.empty() )
            return;
        
        stringstream ss;
        ss << std::chrono::system_clock::now().time_since_epoch().count();
        ss << ".log";
        
        string filename = ss.str();
        
        ofstream myfile;
        myfile.open ( filename );
        
        for(const auto& cmd : commands)
        {
            cout << cmd << endl;
            myfile << cmd << endl;
        }
        
        myfile.close();
    }
};

class StaticBulk
{
public:
    
    StaticBulk( int size ) : max_size( size ) { commands.reserve( max_size ); }
    
    void AddCommand( string&& cmd )
    {
        commands.push_back( move( cmd ) );
        if( static_cast<int>( commands.size() ) == max_size )
        {
            Stop();
        }
    }
    
    void Stop()
    {
        BulkReporter< vector < string > > reporter ( commands );
        Clear();
    }
    
    bool Empty() { return commands.empty(); }
    
private:
    
    void Clear()
    {
        commands.clear();
        commands.reserve( max_size );
    }
    
    vector< string > commands;
    int max_size;
};

class DynamicBulk
{
public:
    
    void AddCommand( string&& cmd )
    {
        commands.push_back( move( cmd ) );
    }
    
    void Stop()
    {
        BulkReporter< list < string > > reporter ( commands );
        Clear();
    }
    
private:
    
    void Clear()
    {
        commands.clear();
    }
    
    list< string > commands;
};

class BulkManager
{
public:
    
    BulkManager( int size ) : static_bulk( size ), dynamic_count( 0 ) {}
    
    void Add( string&& cmd )
    {
        if( cmd == "{" )
        {
            if( !static_bulk.Empty() ) // print static blok if exists
                static_bulk.Stop();
            
            dynamic_count ++;
        }
        else if( cmd == "}" )
        {
            dynamic_count --;
            
            if( dynamic_count == 0 ) // dynamic block is finished
                dynamic_bulk.Stop();
        }
        else
        {
            if( dynamic_count )
                dynamic_bulk.AddCommand( move( cmd ) );
            else
                static_bulk.AddCommand( move ( cmd ) );
        }
    }
    
    void Stop()
    {
        static_bulk.Stop(); // when input data is over only static block needs to be reported
    }
    
private:
    
    StaticBulk static_bulk;
    DynamicBulk dynamic_bulk;
    
    int dynamic_count;
    
};

int main( int /*argc*/, char const *argv[] )
{
    int size = atoi( argv[ 1 ] );
    
    BulkManager bulk_manager( size );
    
    for ( string line; getline( cin, line );)
    {
        bulk_manager.Add( move( line ) );
        sleep( 1 );
    }
    
    bulk_manager.Stop();
    
    return 0;
}
