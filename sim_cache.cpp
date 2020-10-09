#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include <sstream>
#include <math.h>
#include <bitset>
struct CACHE{

        unsigned int BLOCKSIZE = 0; // bytes
        unsigned int L1_SIZE = 0; // bytes
        unsigned int L1_ASSOC = 0; // 1 is direct-mapped
        unsigned int L2_SIZE = 0; // 0 = no L2 cache
        unsigned int L2_ASSOC = 0; // 1 is direct-mapped
        unsigned int REPLACEMENT_POLICY = 0; // 0 = LRU, 1 = PLRU, 2 = optimal
        unsigned int INCLUSION_PROPERTY = 0; // 0 = non-inclusive, 1 = inclusive
        std::string trace_file = ""; // trace file name
    };

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
    }l1_bits, l2_bits;

    struct state {
        
        unsigned int cache_read = 0;
        unsigned int cache_write = 0;
        unsigned int cache_read_miss = 0;
        unsigned int cache_write_miss = 0;
        unsigned int cache_wirteBack = 0;
    }l1_stats,l2_stats;
    unsigned int traffic = 0;
    struct CACHE_WAY{
        std::vector<unsigned int> one_way;
        std::vector<unsigned int> two_way;
        std::vector<unsigned int> third_way;
        std::vector<unsigned int> forth_way;
    }l1_way,l2_way;

    struct RPL_COUNT{
        std::vector<unsigned int> clmn1;
        std::vector<unsigned int> clmn2;
        std::vector<unsigned int> clmn3;
        std::vector<unsigned int> clmn4;
    }l1_rpl,l2_rpl;
    
    // struct to save relevent dirty bits
    struct DIRTY_BIT{
        std::vector<std::string> one_way;
        std::vector<std::string> two_way;
        std::vector<std::string> third_way;
        std::vector<std::string> forth_way;
    }l1_dirtyBit, l2_dirty_bit;
    struct LRU{
        std::vector<unsigned int> one_way;
        std::vector<unsigned int> two_way;
        std::vector<unsigned int> third_way;
        std::vector<unsigned int> forth_way;
    }l1_lru, l2_lru;
    struct addressInBits addBits(unsigned int address_bits, unsigned int tag_bits, unsigned int index_bits);
    // struct split_address extractBit(unsigned int address,unsigned int tagBit,unsigned int indexBit);
    struct BIT calcBit(unsigned int set,unsigned int blocksize);
void L1_access(std::vector<std::string>adr, struct CACHE *cp, struct CACHE_WAY *l1_data, struct RPL_COUNT *s1,
                 unsigned int setNum);

    int main(int argc, char *argv[]) {
    
    char *pCh;
    // start store parameters //
    CACHE my_cache;
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

    
    if(my_cache.L1_ASSOC == 0){
        std::cout << "Minimum associativity value for L1 cache: 1, entered value: " << my_cache.L1_ASSOC << std::endl;
        return 1;
    }
    if(my_cache.L2_SIZE != 0 && my_cache.L2_ASSOC == 0){
        std::cout << "Minimum associativity value for L2 cache: 1, entered value: " << my_cache.L1_ASSOC << std::endl;
        return 1;
    }

    // START find #set, tag bits, index bits, blockoffset //
    L1_cache_set = my_cache.L1_SIZE / (my_cache.L1_ASSOC * my_cache.BLOCKSIZE);

    BIT l2_cache_address;
    CACHE_WAY l2_cache_content;
    if( my_cache.L2_SIZE !=0){
        L2_cache_set = my_cache.L2_SIZE / (my_cache.L2_ASSOC* my_cache.BLOCKSIZE);
       // l2_cache_content = createCache(my_cache.L1_ASSOC, L1_cache_set);
        l2_cache_address = calcBit(L2_cache_set, my_cache.BLOCKSIZE);
        if ((my_cache.BLOCKSIZE & (my_cache.BLOCKSIZE-1) != 0) && (L2_cache_set & (L2_cache_set - 1) != 0))
        {
            std::cout << "Blocksize and set value need to be power of 2 for this simulator." << std::endl;
            return 1; 
        }
           
    }
    
    if ((my_cache.BLOCKSIZE & (my_cache.BLOCKSIZE-1) != 0) && (L1_cache_set & (L1_cache_set - 1) != 0)){
        std::cout << "Blocksize and set value need to be power of 2 for this simulator." << std::endl;
        return 1;
    }
    BIT l1_cache_address = calcBit(L1_cache_set, my_cache.BLOCKSIZE);
    std::cout << "printing address bits" << std::endl;
    std::cout << "index bits: " << l1_cache_address.indexBit << std::endl;
    std::cout << "blockoffseet bits: " << l1_cache_address.blockoffsetBit << std::endl;
    std::cout << "tag bits: " << l1_cache_address.tagBit << std::endl;
    std::cout << "end of address bits" << std::endl;
    // End cache parameters constraint check

    // Resize vector according to set size
    l1_way.one_way.resize(L1_cache_set, 0);
    l1_way.two_way.resize(L1_cache_set, 0);
    l1_way.third_way.resize(L1_cache_set, 0);
    l1_way.forth_way.resize(L1_cache_set, 0);

    // resize vector for locality ref
    l1_rpl.clmn1.resize(L1_cache_set, 0);
    l1_rpl.clmn2.resize(L1_cache_set, 0);
    l1_rpl.clmn3.resize(L1_cache_set, 0);
    l1_rpl.clmn4.resize(L1_cache_set, 0);

    l1_lru.one_way.resize(L1_cache_set, 0);
    l1_lru.one_way.resize(L1_cache_set, 0);
    l1_lru.one_way.resize(L1_cache_set, 0);
    l1_lru.one_way.resize(L1_cache_set, 0);

   
    if(my_cache.L2_SIZE != 0){
        l2_way.one_way.resize(L2_cache_set, 0);
        l2_way.two_way.resize(L2_cache_set, 0);
        l2_way.third_way.resize(L2_cache_set, 0);
        l2_way.forth_way.resize(L2_cache_set, 0);

        l2_rpl.clmn1.resize(L2_cache_set, 0);
        l2_rpl.clmn2.resize(L2_cache_set, 0);
        l2_rpl.clmn3.resize(L2_cache_set, 0);
        l2_rpl.clmn4.resize(L2_cache_set, 0);

        l2_lru.one_way.resize(L2_cache_set, 0);
        l2_lru.one_way.resize(L2_cache_set, 0);
        l2_lru.one_way.resize(L2_cache_set, 0);
        l2_lru.one_way.resize(L2_cache_set, 0);
        }
    // start read from file //
    std::string token="";
    std::vector<std::string> input;
    unsigned int hexAd=0;
    std::fstream myfile (my_cache.trace_file);
    std::string line="";
    
    std::vector<std::string> fileContent;
   unsigned int fileLen=1;
    if(myfile.is_open()){

        while (getline (myfile, line)){
            
            if(line.empty()){
                continue;
            }
            fileContent.push_back(line);
            //std::cout << line << std::endl;
            //std::istringstream stm(line);
            //input.clear();
            //while(stm >> token){
            //    input.push_back(token);
            //}
            
            //std::cout << input[0] << std::endl;
            //std::cout << input[1] << std::endl;
            //hexAd = strtoul (input[1].c_str(), 0, 16);
            //l1_bits = addBits(hexAd, l1_cache_address.tagBit,l1_cache_address.indexBit);
            
            //L1_access(input, &my_cache, &l1_way, l1_cache_address.indexBit,L1_cache_set);
            
            //std::cout << std::hex << "address: "<< l1_bits.adrBits << std::endl;
            //std::cout << std::hex << "tag: " << l1_bits.tBits << std::endl;
            //std::cout << std::dec << "index: " << l1_bits.iBits << std::endl;
            //fileLen++;
            
                    
        }
        //std::cout << "# lines in file: " << fileLen << std::endl;
        //std::cout << "size of cach1 " << sizeof(l1_cache_content) << std::endl;
        //std::cout << "size of cach2" << sizeof(l2_cache_content) << std::endl;
        myfile.close();
    }
    else{
        std::cout << "File doesn't exist." << std::endl;
    }
    for(int i = 0; i<fileContent.size(); i++){
        std::istringstream stm(fileContent[i]);
        input.clear();
        while(stm >> token){
            input.push_back(token);
            }
        std::cout << input[0] << std::endl;
        std::cout << input[1] << std::endl;

        hexAd = strtoul (input[1].c_str(), 0, 16);

        //calculate tag address & index address
        l1_bits = addBits(hexAd, l1_cache_address.tagBit,l1_cache_address.indexBit);
        
        //access cache
        
            
        std::cout << std::hex << "address: "<< l1_bits.adrBits << std::endl;
        std::cout << std::hex << "tag: " << l1_bits.tBits << std::endl;
        std::cout << std::dec << "index: " << l1_bits.iBits << std::endl;
        std::cout << std::hex << l1_way.one_way[l1_cache_address.indexBit] << std::endl;
        L1_access(input, &my_cache, &l1_way, &l1_rpl, L1_cache_set);
        std::cout << std::hex << "l1 cache content at: " << l1_bits.iBits << "\t"
        << l1_way.one_way[l1_bits.iBits] << std::endl;
        if(i == 12){
            break;
        }
    }
    std::cout << "L1 cache read miss: " << l1_stats.cache_read_miss << std::endl;
    std::cout << "L2 cache read: " << l1_stats.cache_read << std::endl;
    std::cout << "L1 cache writes: " << l1_stats.cache_write << std::endl;
    std::cout << "L1 cache write miss: " << l1_stats.cache_write_miss << std::endl;

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
     //std::cout << a.indexValue << std::endl;
    return a;
}*/
 struct addressInBits addBits(unsigned int address_bits,unsigned int tag_bits,unsigned int index_bits){
    addressInBits a;

    std::bitset<32> adr(address_bits);
    std::string mystring = adr.to_string<char,std::string::traits_type,std::string::allocator_type>();
    a.adrBits = adr.to_ulong();
    //std::cout << "address: " << adr << std::endl;
    //std::cout << std::hex << "address in hex: " << a.adrBits << std::endl;

    std::bitset<32> adrTag(mystring,0,tag_bits); 
    //std::cout << "tag: " << adrTag << std::endl;
    a.tBits = adrTag.to_ulong();
    //std::cout << std::hex << "tag in hex: " << a.tBits << std::endl;a
    
    std::bitset<32> adrIdx(mystring,22,6);
    //std::cout << "idx: " << adrIdx << std::endl;
    a.iBits = adrIdx.to_ulong();
    //std::cout << std::hex << "idx in hex: " << a.iBits << std::endl;
    return a;
    //std::cout << "tag bits: " << add2 << std::endl;

 }


void L1_access(std::vector<std::string>adr, struct CACHE *cp, struct CACHE_WAY *l1_data, struct RPL_COUNT *s1, unsigned int setNum){
    
    if(adr[0] == "r"){
        
        if(l1_data->one_way[l1_bits.iBits] == 0){
            
            l1_data->one_way[l1_bits.iBits] = l1_bits.tBits;
            l1_stats.cache_read_miss++;
            // moidfy lru counter
             if(cp->REPLACEMENT_POLICY == 0){
                l1_lru.one_way[l1_bits.iBits] += 1;
                }
            // access l2 cache function
            
        }

        else if(l1_bits.tBits == l1_data->one_way[l1_bits.iBits]){
            l1_stats.cache_read++;
            // moidfy lru counter
            if(cp->REPLACEMENT_POLICY == 0){
                l1_lru.one_way[l1_bits.iBits] += 1;
                }
        }
        else if(l1_bits.tBits != l1_data->one_way[l1_bits.iBits] && l1_dirtyBit.one_way[l1_bits.iBits] == "D"){

            l1_stats.cache_read_miss++;
            // call l2 cache
            //also perform replacement policy
            // inclusion property function
            //if dirty bit writeback
        }
    }
    else if(adr[0] == "w"){
        if(l1_data->one_way[l1_bits.iBits] == 0){
            l1_stats.cache_write_miss++;
            l1_data->one_way[l1_bits.iBits] = l1_bits.tBits;
            
            // moidfy lru counter
            if(cp->REPLACEMENT_POLICY == 0){
                l1_lru.one_way[l1_bits.iBits] += 1;
            }
        }
        else if (l1_data->one_way[l1_bits.iBits] == l1_bits.tBits){
            l1_dirtyBit.one_way[l1_bits.iBits] = "D";

            // moidfy lru counter
            if(cp->REPLACEMENT_POLICY == 0){
                l1_lru.one_way[l1_bits.iBits] += 1;
            }

        }
        else if(l1_data->one_way[l1_bits.iBits] != l1_bits.tBits && cp->INCLUSION_PROPERTY == 0){
            if(l1_dirtyBit.one_way[l1_bits.iBits] != "D"){
                if(cp->REPLACEMENT_POLICY == 0){
                     l1_data->one_way[l1_bits.iBits] = l1_bits.tBits;
                }

                if(cp->REPLACEMENT_POLICY == 1){
                     // PLRU
                }

                else if(cp->REPLACEMENT_POLICY == 2){
                     //OPTIMAL
                }
            }

            else if(l1_dirtyBit.one_way[l1_bits.iBits] == "D"){
                // compare lru vectors and replace vector element with least value
                // also adjust vector lru count with same vector[i]
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