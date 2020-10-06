#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include <sstream>
#include <math.h>
#include <bitset>
struct CACHE{

        unsigned int BLOCKSIZE, // bytes
         L1_SIZE, // bytes
         L1_ASSOC, // 1 is direct-mapped
         L2_SIZE, // 0 = no L2 cache
         L2_ASSOC, // 1 is direct-mapped
         REPLACEMENT_POLICY, // 0 = LRU, 1 = PLRU, 2 = optimal
         INCLUSION_PROPERTY=0; // 0 = non-inclusive, 1 = inclusive
        std::string trace_file=""; // trace file name
    }my_cache;

    struct BIT {
        unsigned int indexBit=0;
        unsigned int blockoffsetBit=0;
        unsigned int tagBit=0;
    };
    struct split_address {
        unsigned int tagValue=0;
        unsigned int indexValue=0;
    };

    struct addressInBits{
        unsigned int adrBits=0;
        unsigned int tBits=0;
        unsigned int iBits=0;
    };

    struct state {
        unsigned int traffic = 0;
        unsigned int cache_read = 0;
        unsigned int cache_write = 0;
        unsigned int cache_read_miss = 0;
        unsigned int cache_write_miss = 0;
        unsigned int cache_wirteBack = 0;
    }l1_stats,l2_stats;

    struct CACHE_WAY{
        std::vector<std::string> one_way;
        std::vector<std::string> two_way;
        std::vector<std::string> third_way;
        std::vector<std::string> forth_way;
    };
    struct addressInBits addBits(unsigned int address_bits, unsigned int tag_bits, unsigned int index_bits);
    // struct split_address extractBit(unsigned int address,unsigned int tagBit,unsigned int indexBit);
    struct BIT calcBit(unsigned int set,unsigned int blocksize);
void L1_access(std::string oprt, std::string adr, unsigned int asoc,unsigned int setNum, unsigned int rpl, unsigned int incProp, std::vector<std::string> wayOne);
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
    CACHE_WAY l1_cache_content = createCache(my_cache.L1_ASSOC, L1_cache_set);

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
    
    BIT l2_cache_address;
    CACHE_WAY l2_cache_content;
    if( my_cache.L2_SIZE !=0){
        L2_cache_set = my_cache.L2_SIZE / (my_cache.L2_ASSOC* my_cache.BLOCKSIZE);
        l2_cache_content = createCache(my_cache.L1_ASSOC, L1_cache_set);
        l2_cache_address = calcBit(L2_cache_set, my_cache.BLOCKSIZE);
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
    BIT l1_cache_address = calcBit(L1_cache_set, my_cache.BLOCKSIZE);
    std::cout << "printing address bits" << "\n";
    std::cout << "index bits: " << l1_cache_address.indexBit << "\n";
    std::cout << "blockoffseet bits: " << l1_cache_address.blockoffsetBit << "\n";
    std::cout << "tag bits: " << l1_cache_address.tagBit << "\n";
    std::cout << "end of address bits" << "\n";
    // End cache parameters constraint check

    //std::cout << "you entered: " << argc << "arguments: " << "\n";
    //for(int i = 1; i <= argc; ++i){
      //  std::cout << argv[i] << "\t";
    //}
    //std::cout << L1_cache_set << "\n";
    //std::cout << L2_cache_set << "\n";

    // start read from file //
    std::string token="";
    char address[8];
    std::vector<std::string> input;
    unsigned int hexAd=0;
    std::fstream myfile (my_cache.trace_file);
    std::string line="";

    //split_address l1_decode;
    //split_address l2_decode;
    addressInBits l1_bits;
    //std::string addressInBits;
   unsigned int fileLen=1;
    if(myfile.is_open()){
        while (getline (myfile, line)){
            
            if(line.empty()){
                continue;
            }
            std::cout << line << "\n";
            std::istringstream stm(line);
            input.clear();
            while(stm >> token){
                input.push_back(token);
            }
            
            std::cout << input[0] << "\n";
            std::cout << input[1] << "\n";
            hexAd = strtoul (input[1].c_str(), 0, 16);
            l1_bits = addBits(hexAd, l1_cache_address.tagBit,l1_cache_address.indexBit);
           // L1_access(input[0],input[1], my_cache.L1_ASSOC,my_cache.REPLACEMENT_POLICY, my_cache.INCLUSION_PROPERTY);
            std::cout << std::hex << "address: "<< l1_bits.adrBits << "\n";
            std::cout << std::hex << "tag: " << l1_bits.tBits << "\n";
            std::cout << std::dec << "index: " << l1_bits.iBits << "\n";
            fileLen++;
            
                    
        }
        std::cout << "# lines in file: " << fileLen << "\n";
        std::cout << "size of cach1 " << sizeof(l1_cache_content) << "\n";
        std::cout << "size of cach2" << sizeof(l2_cache_content) << "\n";
        myfile.close();
        //std::cout << hexAd << "\n";
        //std::cout << std::hex << l1_bits.adrBits << "\n";
        //std::cout << std::hex << l1_bits.iBits << "\n";
        //std::cout << std::hex << l1_bits.tBits << "\n";
    }
    else{
        std::cout << "File doesn't exist." << "\n";
    }
    
    return 0;
}

struct BIT calcBit(unsigned int set,unsigned int blocksize){
        struct BIT bit;
        bit.indexBit = log2 (set);
        bit.blockoffsetBit = log2 (blocksize);
        bit.tagBit = 32 - bit.indexBit - bit.blockoffsetBit;
        struct BIT a = bit;
        return a;     
    }

/*struct split_address extractBit(unsigned int address,unsigned int tagBit,unsigned int indexBit){
    struct split_address a;
    unsigned int mask;
    mask = ((1 << tagBit) - 1) << 1;
    a.tagValue = address & mask;
    unsigned int indxAdd = address - a.tagValue;
    mask = ((1 << indexBit) - 1) << 1;
    a.indexValue = indxAdd & mask;
     //std::cout << a.indexValue << "\n";
    return a;
}*/
 struct addressInBits addBits(unsigned int address_bits,unsigned int tag_bits,unsigned int index_bits){
    addressInBits a;

    std::bitset<32> adr(address_bits);
    std::string mystring = adr.to_string<char,std::string::traits_type,std::string::allocator_type>();
    a.adrBits = adr.to_ulong();
    std::cout << "address: " << adr << "\n";
    //std::cout << std::hex << "address in hex: " << a.adrBits << "\n";

    std::bitset<32> adrTag(mystring,0,tag_bits); 
    std::cout << "tag: " << adrTag << "\n";
    a.tBits = adrTag.to_ulong();
    //std::cout << std::hex << "tag in hex: " << a.tBits << "\n";a
    
    std::bitset<32> adrIdx(mystring,22,6);
    std::cout << "idx: " << adrIdx << "\n";
    a.iBits = adrIdx.to_ulong();
    //std::cout << std::hex << "idx in hex: " << a.iBits << "\n";
    return a;
    //std::cout << "tag bits: " << add2 << "\n";

 }

/*struct CACHE_WAY createCache(unsigned int asoc, unsigned int set){
    CACHE_WAY content;
    if (asoc==1){
        
        content.one_way;

    }
    else if(asoc == 2){
        
        content.one_way;
        content.two_way;
    }
     else if(asoc == 2){
         
        content.one_way;
        content.two_way;
        content.third_way;
        content.forth_way;
    }
    return content;
}
*/

void L1_access(std::string oprt, std::string adr, unsigned int asoc,unsigned int setNum, unsigned int rpl, unsigned int incProp, std::vector<std::string> wayOne){
    int i = 0;
    while (i < setNum){
        if(adr == wayOne[i]){
            if(oprt == "r"){
                l1_stats.cache_read++;
                
            }
            else if(oprt == "w"){
                l1_stats.cache_write++;
            }
        }
    }

    }

/*
0100000000000001001101 1000001100
0100000000000001001101 1000001100
00000000000100000000000001001101
00000000000100000000000001001101
00000000001000000000000010011011

10111 0100
*/