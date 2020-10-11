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
    }l1_bits, l2_bits;
    /*
    struct split_address {
        unsigned int tagValue=0;
        unsigned int indexValue=0;
    };
    */
    struct addressInBits{
        unsigned int adrBits=0;
        unsigned int tBits=0;
        unsigned int iBits=0;
    }l1_field, l2_field;

    struct state {
        
        unsigned int cache_read = 0;
        unsigned int cache_write = 0;
        unsigned int cache_read_miss = 0;
        unsigned int cache_write_miss = 0;
        unsigned int cache_write_back = 0;
    }l1_stats,l2_stats;
    unsigned int traffic = 0; // track main mem access
    struct CACHE_WAY{
        std::vector<unsigned int> one_way;
        std::vector<unsigned int> two_way;
        std::vector<unsigned int> third_way;
        std::vector<unsigned int> forth_way;
    }l1_way,l2_way;

     struct l1_l2_ref{
        std::vector<unsigned int> one_way;
        std::vector<unsigned int> two_way;
        std::vector<unsigned int> third_way;
        std::vector<unsigned int> forth_way;
    }l2_ref;

    struct VALIDITY{
        std::vector<std::string> clmn1;
        std::vector<std::string> clmn2;
        std::vector<std::string> clmn3;
        std::vector<std::string> clmn4;
    }l1_vld,l2_vld;
    
    // struct to save relevent dirty bits
    struct DIRTY_BIT{
        std::vector<std::string> one_way;
        std::vector<std::string> two_way;
        std::vector<std::string> third_way;
        std::vector<std::string> forth_way;
    }l1_dirty_bit, l2_dirty_bit;
    struct LRU{
        std::vector<unsigned int> one_way;
        std::vector<unsigned int> two_way;
        std::vector<unsigned int> third_way;
        std::vector<unsigned int> forth_way;
    }l1_lru, l2_lru;

    struct addressInBits addBits(unsigned int address_bits, unsigned int tag_bits, unsigned int index_bits);
    
    struct BIT calcBit(unsigned int set,unsigned int blocksize);
    
    void L1_access(std::vector<std::string>adr, struct CACHE *cp, unsigned int setNum);


    // START main //
    int main(int argc, char *argv[]) {
    
    char *pCh;
    // start store cache parameters //
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
    // end store cache parameters //

    
    // start check input constraints
    if ((my_cache.BLOCKSIZE & (my_cache.BLOCKSIZE-1) != 0) && (L1_cache_set & (L1_cache_set - 1) != 0)){
        std::cout << "Blocksize and set value need to be power of 2 for this simulator." << std::endl;
        return 1;
    }

    if(my_cache.L1_ASSOC == 0){
        std::cout << "Minimum associativity value for L1 cache: 1, entered value: " << my_cache.L1_ASSOC << std::endl;
        return 1;
    }

    if(my_cache.L2_SIZE != 0 && my_cache.L2_ASSOC == 0){
        std::cout << "Minimum associativity value for L2 cache: 1, entered value: " << my_cache.L1_ASSOC << std::endl;
        return 1;
    }

    if( my_cache.L2_SIZE !=0){
        L2_cache_set = my_cache.L2_SIZE / (my_cache.L2_ASSOC* my_cache.BLOCKSIZE);
        l2_bits = calcBit(L2_cache_set, my_cache.BLOCKSIZE);
        if ((my_cache.BLOCKSIZE & (my_cache.BLOCKSIZE-1) != 0) && (L2_cache_set & (L2_cache_set - 1) != 0))
        {
            std::cout << "Blocksize and set value need to be power of 2 for this simulator." << std::endl;
            return 1; 
        }
           
    }
    // end check input constraints

    // start find # of sets, tag bits, index bits, blockoffset //
    L1_cache_set = my_cache.L1_SIZE / (my_cache.L1_ASSOC * my_cache.BLOCKSIZE);

    // calcualte # of bits per address field
    l1_bits = calcBit(L1_cache_set, my_cache.BLOCKSIZE);

    if(my_cache.L2_SIZE != 0){
        l2_bits = calcBit(L2_cache_set, my_cache.BLOCKSIZE);
    }
    std::cout << "printing address bits" << std::endl;
    std::cout << "index bits: " << l1_bits.indexBit << std::endl;
    std::cout << "blockoffseet bits: " << l1_bits.blockoffsetBit << std::endl;
    std::cout << "tag bits: " << l1_bits.tagBit << std::endl;
    std::cout << "end of address bits" << std::endl;

    // start Resize vector according to set size
    l1_way.one_way.resize(L1_cache_set, 0);
    l1_way.two_way.resize(L1_cache_set, 0);
    l1_way.third_way.resize(L1_cache_set, 0);
    l1_way.forth_way.resize(L1_cache_set, 0);
    
    l1_dirty_bit.one_way.resize(L1_cache_set, " ");
    l1_dirty_bit.two_way.resize(L1_cache_set, " ");
    l1_dirty_bit.third_way.resize(L1_cache_set, " ");
    l1_dirty_bit.forth_way.resize(L1_cache_set, " ");

    l1_lru.one_way.resize(L1_cache_set, 0);
    l1_lru.one_way.resize(L1_cache_set, 0);
    l1_lru.one_way.resize(L1_cache_set, 0);
    l1_lru.one_way.resize(L1_cache_set, 0);

        l1_vld.clmn1.resize(L1_cache_set, "0");
        l1_vld.clmn2.resize(L1_cache_set, "0");
        l1_vld.clmn3.resize(L1_cache_set, "0");
        l1_vld.clmn4.resize(L1_cache_set, "0");
   
    if(my_cache.L2_SIZE != 0){
        l2_way.one_way.resize(L2_cache_set, 0);
        l2_way.two_way.resize(L2_cache_set, 0);
        l2_way.third_way.resize(L2_cache_set, 0);
        l2_way.forth_way.resize(L2_cache_set, 0);

        l2_lru.one_way.resize(L2_cache_set, 0);
        l2_lru.one_way.resize(L2_cache_set, 0);
        l2_lru.one_way.resize(L2_cache_set, 0);
        l2_lru.one_way.resize(L2_cache_set, 0);

        l2_ref.one_way.resize(L2_cache_set, 0);
        l2_ref.one_way.resize(L2_cache_set, 0);
        l2_ref.one_way.resize(L2_cache_set, 0);
        l2_ref.one_way.resize(L2_cache_set, 0);
        
        

        l2_dirty_bit.one_way.resize(L2_cache_set, " ");
        l2_dirty_bit.two_way.resize(L2_cache_set, " ");
        l2_dirty_bit.third_way.resize(L2_cache_set, " ");
        l2_dirty_bit.forth_way.resize(L2_cache_set, " ");

        }
    // end resize vector according to set size

    // start get trace file data. //
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
                    
        }
        
        myfile.close();
    }
    else{
        std::cout << "File doesn't exist." << std::endl;
    }
    std::cout << std::hex << fileContent[0] << std::endl;
    fileContent[0].erase(0,3);
    std::cout << std::hex << fileContent[0] << std::endl;
    // end get file data //

    // start cache access process //
    std::cout << std::dec << "file size: " << fileContent.size() << std::endl;
    for(int i = 0; i<fileContent.size(); i++){
        //std::cout<< std::dec << "fetching address at line # " << i << std::endl; 
        std::istringstream stm(fileContent[i]);
        input.clear();
        while(stm >> token){
            input.push_back(token);
            }
        //std::cout << input[0] << std::endl;
        //std::cout << input[1] << std::endl;

        hexAd = strtoul (input[1].c_str(), 0, 16);

        //calculate tag address & index address
        l1_field = addBits(hexAd, l1_bits.tagBit,l1_bits.indexBit);
        if(my_cache.L2_SIZE != 0){
            l2_field = addBits(hexAd, l2_bits.tagBit,l2_bits.indexBit);
        }
        //std::cout << std::dec << " address line # " << i << std::endl;        
        //std::cout << std::hex << "address: "<< l1_field.adrBits << std::endl;
        //std::cout << std::hex << "tag: " << l1_field.tBits << std::endl;
        //std::cout << std::dec << "index: " << l1_field.iBits << std::endl;
        
        
        L1_access(input, &my_cache, L1_cache_set);
        //std::cout << std::dec << "l1 cache content at: " << l1_field.iBits << "\t" << l1_way.one_way[l1_field.iBits] << std::endl;
        /*if(i == 12){
            break;
        }*/
    }
    // end cache access process //
    std::cout << "===== L1 contents =====" << std::endl;
    for(int i = 0; i<l1_way.one_way.size();i++){
        std::cout << std::dec << "Set\t" << i << ":\t";
        std::cout << std::hex << l1_way.one_way[i] << "\t" << l1_dirty_bit.one_way[i] << std::endl; 
    }
    std::cout << std::dec << "===== Simulation results (raw) =====" << std::endl;
    std::cout << std::dec << "a." << "L1 cache read miss: " << l1_stats.cache_read_miss << std::endl;
    std::cout << std::dec << "b." << "L2 cache read: " << l1_stats.cache_read << std::endl;
    std::cout << std::dec << "c." << "L1 cache writes: " << l1_stats.cache_write << std::endl;
    std::cout << std::dec << "c." << "L1 cache write miss: " << l1_stats.cache_write_miss << std::endl;
    std::cout << std::dec << "d." << "L1 cache writebacks: " << l1_stats.cache_write_back << std::endl;
    std::cout << std::dec << "d." << "total memory traffic: " << traffic << std::endl;

     return 0;
}

// END main //

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


    std::bitset<32> adrTag(mystring,0,tag_bits); 
 
    a.tBits = adrTag.to_ulong();
    
    
    std::bitset<32> adrIdx(mystring,22,6);
    
    a.iBits = adrIdx.to_ulong();

    return a;
  

 }


void L1_access(std::vector<std::string>adr, struct CACHE *cp, unsigned int setNum){
     // operation read //
     int ref = 0;
    if(adr[0] == "r"){


        // HIT CASE
        if(l1_field.tBits == l1_way.one_way[l1_field.iBits] ){
            //std::cout << std::dec << "***cache read hit***" << std::endl;
            //std::cout << std::hex << "****" << l1_way.one_way[l1_field.iBits] << "****" << std::endl;
            //std::cout << std::hex << "****" << l1_field.tBits << "****" << std::endl;
            l1_stats.cache_read++;
            // moidfy lru counter
            if(cp->REPLACEMENT_POLICY == 0 ){
                l1_lru.one_way[l1_field.iBits] += 1;
                }
        }

        // MISS CASE
        else if(l1_field.tBits != l1_way.one_way[l1_field.iBits]){
            if(l1_dirty_bit.one_way[l1_field.iBits] == "D"){
                l1_stats.cache_write_back++;
                l1_dirty_bit.one_way[l1_field.iBits] = " ";
                traffic++; //traffic because writeback
            }
            l1_stats.cache_read_miss++;
            traffic++;
            if (cp->L2_SIZE == 0){
                if(l1_vld.clmn1[l1_field.iBits] == "invalid"){
                            l1_vld.clmn1[l1_field.iBits] = " ";
                        }
                l1_way.one_way[l1_field.iBits] = l1_field.tBits;
                l1_stats.cache_read++;
                
            }
            else if(cp->L2_SIZE !=0){
                if(l2_way.one_way[l2_field.iBits] == 0){
                    l2_stats.cache_read_miss++;
                    l1_stats.cache_read++;
                    l2_way.one_way[l2_field.iBits] = l2_field.tBits;
                    if(l1_vld.clmn1[l1_field.iBits] == "invalid"){
                            l1_vld.clmn1[l1_field.iBits] = " ";
                        }
                    l1_way.one_way[l1_field.iBits] = l1_field.tBits;
                    traffic++; //traffic because read
            // moidfy lru counter
                    if(cp->REPLACEMENT_POLICY == 0){
                        l2_lru.one_way[l2_field.iBits] += 1;
                        l1_lru.one_way[l1_field.iBits] += 1;
                    }
                }

                else if(l2_field.tBits == l2_way.one_way[l2_field.iBits]){
                    l2_stats.cache_read++;
                    // moidfy lru counter
                    if(l1_vld.clmn1[l1_field.iBits] == "invalid"){
                            l1_vld.clmn1[l1_field.iBits] = " ";
                        }
                    l1_way.one_way[l1_field.iBits] = l1_field.tBits;
                    if(cp->REPLACEMENT_POLICY == 0 ){
                        l2_lru.one_way[l2_field.iBits] += 1;
                        l1_lru.one_way[l1_field.iBits] += 1;
                    }
                }

                else if(l2_field.tBits != l2_way.one_way[l2_field.iBits]){
                    traffic++;
                    l2_stats.cache_read_miss++;
                    l1_stats.cache_read++;
                    if(l2_dirty_bit.one_way[l2_field.iBits] == "D"){
                        l2_dirty_bit.one_way[l2_field.iBits] = " ";
                        l2_stats.cache_write_back++;
                        traffic++;
                    }
                    
                    if(cp->INCLUSION_PROPERTY == 0){
                        l1_way.one_way[l1_field.iBits] = l1_field.iBits;
                        l2_way.one_way[l2_field.iBits] = l2_field.tBits;
                        
                    }
                    else if(cp->INCLUSION_PROPERTY == 1){
                        for(int i = 0; i < l1_way.one_way.size(); i++){
                            if(l1_way.one_way[i] == l2_ref.one_way[l2_field.iBits]){
                                ref = i;
                                l1_vld.clmn1[i] = "invalid";
                                if(l1_dirty_bit.one_way[i] == "D"){
                                    l1_stats.cache_write_back++;
                                    l1_dirty_bit.one_way[i] = " ";
                                }
                                l1_way.one_way[i] = 0;
                                break;
                            }
                        }
                        
                        l2_way.one_way[l2_field.iBits] = l2_field.tBits;
                        l2_ref.one_way[l2_field.iBits] = l2_field.tBits; // saving l1 tag to use when setting invalid...
                        if(l1_vld.clmn1[l1_field.iBits] == "invalid"){
                            l1_vld.clmn1[l1_field.iBits] = " ";
                        }
                        l1_way.one_way[l1_field.iBits] = l1_field.tBits;
                        
                    }
                }
        

            }
        }

    }

    // WRTIE OPERATION
    else if(adr[0] == "w"){
        
        // HIT CASE
        if(l1_field.tBits == l1_way.one_way[l1_field.iBits] ){
            //std::cout << std::dec << "***cache read hit***" << std::endl;
            //std::cout << std::hex << "****" << l1_way.one_way[l1_field.iBits] << "****" << std::endl;
            //std::cout << std::hex << "****" << l1_field.tBits << "****" << std::endl;
            l1_stats.cache_write++;
            l1_dirty_bit.one_way[l1_field.iBits] = "D";
            // moidfy lru counter
            if(cp->REPLACEMENT_POLICY == 0 ){
                l1_lru.one_way[l1_field.iBits] += 1;
                }
        }

        // MISS CASE
            else if(l1_field.tBits != l1_way.one_way[l1_field.iBits]){
            if(l1_dirty_bit.one_way[l1_field.iBits] == "D"){
                l1_stats.cache_write_back++;
                l1_dirty_bit.one_way[l1_field.iBits] = " ";
                traffic++; //traffic because writeback
            }
            l1_stats.cache_write_miss++;
            traffic++;
            if (cp->L2_SIZE == 0){
                if(l1_vld.clmn1[l1_field.iBits] == "invalid"){
                            l1_vld.clmn1[l1_field.iBits] = " ";
                        }
                l1_way.one_way[l1_field.iBits] = l1_field.tBits;
                
                l1_stats.cache_write++;
                l1_dirty_bit.one_way[l1_field.iBits] = "D";
            }
            else if(cp->L2_SIZE !=0){
                if(l2_way.one_way[l2_field.iBits] == 0){
                    l2_dirty_bit.one_way[l2_field.iBits] = "D";
                    l2_stats.cache_write_miss++;
                    l1_stats.cache_write++;
                    l2_way.one_way[l2_field.iBits] = l2_field.tBits;
                    if(l1_vld.clmn1[l1_field.iBits] == "invalid"){
                            l1_vld.clmn1[l1_field.iBits] = " ";
                        }
                    l1_way.one_way[l1_field.iBits] = l1_field.tBits;
                    traffic++; //traffic because read
            // moidfy lru counter
                    if(cp->REPLACEMENT_POLICY == 0){
                        l2_lru.one_way[l2_field.iBits] += 1;
                        l1_lru.one_way[l1_field.iBits] += 1;
                    }
                }

                else if(l2_field.tBits == l2_way.one_way[l2_field.iBits]){
                    l2_dirty_bit.one_way[l2_field.iBits] = "D";
                    l2_stats.cache_write++;
                    // moidfy lru counter
                    if(l1_vld.clmn1[l1_field.iBits] == "invalid"){
                            l1_vld.clmn1[l1_field.iBits] = " ";
                        }
                    l1_way.one_way[l1_field.iBits] = l1_field.tBits;
                    if(cp->REPLACEMENT_POLICY == 0 ){
                        l2_lru.one_way[l2_field.iBits] += 1;
                        l1_lru.one_way[l1_field.iBits] += 1;
                    }
                }

                else if(l2_field.tBits != l2_way.one_way[l2_field.iBits]){
                    traffic++;
                    l2_stats.cache_write_miss++;
                    l1_stats.cache_write++;
                    if(l2_dirty_bit.one_way[l2_field.iBits] == "D"){
                        l2_dirty_bit.one_way[l2_field.iBits] == " ";
                        l2_stats.cache_write_back++;
                        traffic++;
                    }
                    
                    if(cp->INCLUSION_PROPERTY == 0){
                        l1_way.one_way[l1_field.iBits] = l1_field.iBits;
                        l2_way.one_way[l2_field.iBits] = l2_field.tBits;
                        
                    }
                    else if(cp->INCLUSION_PROPERTY == 1){
                        for(int i = 0; i < l1_way.one_way.size(); i++){
                            if(l1_way.one_way[i] == l2_ref.one_way[l2_field.iBits]){
                                ref = i;
                                l1_vld.clmn1[i] = "invalid";
                                if(l1_dirty_bit.one_way[i] == "D"){
                                    l1_stats.cache_write_back++;
                                    l1_dirty_bit.one_way[i] = " ";
                                }
                                l1_way.one_way[i] = 0;
                                break;
                            }
                        }
                        
                        l2_way.one_way[l2_field.iBits] = l2_field.tBits;
                        l2_ref.one_way[l2_field.iBits] = l2_field.tBits; // saving l1 tag to use when setting invalid...
                        if(l1_vld.clmn1[l1_field.iBits] == "invalid"){
                            l1_vld.clmn1[l1_field.iBits] = " ";
                        }
                        l1_way.one_way[l1_field.iBits] = l1_field.tBits;
                        
                    }
                }
        

            }
        }

    }
        
}

/*
100000000000000101110
100000000000000101110
*/