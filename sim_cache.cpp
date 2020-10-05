#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include <sstream>
#include <math.h>

struct CACHE{

        unsigned int BLOCKSIZE, // bytes
         L1_SIZE, // bytes
         L1_ASSOC, // 1 is direct-mapped
         L2_SIZE, // 0 = no L2 cache
         L2_ASSOC, // 1 is direct-mapped
         REPLACEMENT_POLICY, // 0 = LRU, 1 = PLRU, 2 = optimal
         INCLUSION_PROPERTY; // 0 = non-inclusive, 1 = inclusive
        std::string trace_file; // trace file name
    }my_cache;

    struct BIT {
        unsigned int indexBit;
        unsigned int blockoffsetBit;
        unsigned int tagBit;
    };
    struct BIT calcBit(int set, int blocksize);
    int main(int argc, char *argv[]) {
    
    char *pCh;
    // start store parameters //
    unsigned int L1_cache_set, L2_cache_set;
    my_cache.BLOCKSIZE = strtoul (argv[1], &pCh, 10);
    my_cache.L1_SIZE = strtoul (argv[2], &pCh, 10);
    my_cache.L1_ASSOC =strtoul (argv[3], &pCh, 10);
    my_cache.L2_SIZE = strtoul (argv[4], &pCh, 10);
    my_cache.L2_ASSOC =  strtoul (argv[5], &pCh, 10);
    my_cache.REPLACEMENT_POLICY = strtoul (argv[6], &pCh, 10);
    my_cache.INCLUSION_PROPERTY = strtoul (argv[7], &pCh, 10);
    my_cache.trace_file = argv[8];
    // end store parameters //

    // START find #set, tag bits, index bits, blockoffset //
    
    L1_cache_set = my_cache.L1_SIZE / (my_cache.L1_ASSOC* my_cache.BLOCKSIZE);
    //unsigned int indexBit = log2 (L1_cache_set);
    //unsigned int blockoffsetBit = log2(my_cache.BLOCKSIZE);
    //unsigned int tagBit = 32 - indexBit - blockoffsetBit;
    
    // END find #set, tag bits, index bits, blockoffset //
    
    // start check cacahe parameter consraints //
    if(my_cache.L1_ASSOC == 0){
        std::cout << "Minimum associativity value for L1 cache: 1, entered value: " << my_cache.L1_ASSOC << "\n";
        return 1;
    }
    if(my_cache.L2_SIZE != 0 && my_cache.L2_ASSOC == 0){
        std::cout << "Minimum associativity value for L2 cache: 1, entered value: " << my_cache.L1_ASSOC << "\n";
        return 1;
    }
    
    struct BIT l2_cache_address;

    if( my_cache.L2_SIZE !=0){
        L2_cache_set = my_cache.L2_SIZE / (my_cache.L2_ASSOC* my_cache.BLOCKSIZE);
        l2_cache_address = calcBit(L1_cache_set, my_cache.BLOCKSIZE);
        if ((my_cache.BLOCKSIZE & (my_cache.BLOCKSIZE-1) != 0) && (L2_cache_set & (L2_cache_set - 1) != 0))
        {
            std::cout << "Blocksize and set value need to be power of 2 for this simulator.";
            return 1; 
        }
           
    }
    
    if ((my_cache.BLOCKSIZE & (my_cache.BLOCKSIZE-1) != 0) && (L1_cache_set & (L1_cache_set - 1) != 0)){
        std::cout << "Blocksize and set value need to be power of 2 for this simulator.";
        return 1;
    }
    struct BIT l1_cache_address = calcBit(L1_cache_set, my_cache.BLOCKSIZE);
    // End cache parameters constraint check

    //std::cout << "you entered: " << argc << "arguments: " << "\n";
    //for(int i = 1; i <= argc; ++i){
      //  std::cout << argv[i] << "\t";
    //}
    std::cout << L1_cache_set << "\n";
    std::cout << L2_cache_set << "\n";

    // start read from file //
    std::string token;
    char address[8];
    std::vector<std::string> input;
    unsigned int hexAd;
    std::fstream myfile (my_cache.trace_file);
    std::string line;
    if(myfile.is_open()){
        while (getline (myfile, line)){
            if(line.empty()){
                continue;
            }
            std::cout << line << "\n";
            std::istringstream stm(line);
            while(stm >> token){
                input.push_back(token);
            }
            
            std::cout << input[0] << "\n";
            std::cout << input[1] << "\n";
            hexAd = strtoul (input[1].c_str(), 0, 16);
            break;
                    
        }
        myfile.close();
        std::cout << hexAd << "\n";
    }
    else std::cout << "File doesn't exist.";
     // end read from file //

     // start extract tag and index bits //
        unsigned  mask;
    //mask = (1 << X) - 1;
    //lastXbits = value & mask;
     // end extract tag and index bits //
    return 0;
}

struct BIT calcBit(int set, int blocksize){
        struct BIT bit;
        bit.indexBit = log2 (set);
        bit.blockoffsetBit = log2 (blocksize);
        bit.tagBit = 32 - bit.indexBit - bit.blockoffsetBit;
        struct BIT a = bit;
        return a;     
    }