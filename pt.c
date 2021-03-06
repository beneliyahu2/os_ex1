//
// Operation Systems Course Exercise 1
// Avigail Ben Eliyahu
// ID 305703720
// 30-Oct-21.
//
#include "os.h"

typedef struct{
    uint64_t *pte_ptr;
    int level;
    int valid;
} pte_loc;

pte_loc find(uint64_t pt, uint64_t vpn){
    pte_loc location;
    uint64_t* first_node = (uint64_t*)phys_to_virt(pt << 12);
    uint64_t* curr_node = first_node;
    uint64_t curr_symbol;
    uint64_t curr_pte;
    uint64_t valid_bit;
    for (int i = 0; i<5; i++){
        curr_symbol = (vpn << (19+9*i)) >> 55;
        curr_pte = curr_node[curr_symbol];
        valid_bit = curr_pte & 1;
        location.level = i;
        location.pte_ptr = &(curr_node[curr_symbol]);
        if (valid_bit == 0){
            location.valid = 0;
            break;
        }
        else{ // if valid_bit is 1:
            location.valid = 1;
            if (i == 4){
                break;
            }
            else{ //moving to the next node:
                curr_node = (uint64_t*)phys_to_virt(curr_pte & (~1)); //changing valid bit to 0 before converting to virtual.
            }
        }
    }
    return location;
}

void map_rest_of_path(uint64_t vpn, uint64_t ppn, pte_loc location){
    uint64_t* curr_pte_ptr = location.pte_ptr;
    uint64_t* next_node_ptr;
    uint64_t next_symbol;
    for (int i = location.level; i<4;  i++){ // updating all the inner nodes
        uint64_t next_node_ppn = alloc_page_frame();
        uint64_t phys_addr = (next_node_ppn << 12); // adding 12 zeros to the right
        *curr_pte_ptr = (phys_addr | 1); // updating the current pte (in node i) and changing valid bit to 1;
        next_node_ptr = (uint64_t*)phys_to_virt(phys_addr); // pointer to the next node
        next_symbol = ((vpn << (19+9*(i+1))) >> 55);
        curr_pte_ptr = &(next_node_ptr[next_symbol]); // which is actually the "next pte pointer" not the current
    }
    *curr_pte_ptr = ((ppn << 12) | 1); // updating the leave (node number 4)
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn){
    pte_loc location = find(pt, vpn);
    if (location.valid == 1){
        uint64_t ppn = ((*(location.pte_ptr)) >> 12);
        return ppn;
    }
    return NO_MAPPING;
}

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn){
    pte_loc location = find(pt, vpn);
    if (ppn == NO_MAPPING){ // User asked to erase mapping:
        if (location.valid == 0){
            return;
        } else{
            *(location.pte_ptr) = ((*(location.pte_ptr)) & (~1)); //changing valid bit to 0 (in the last node)
        }
    }
    else{ // User asked to map to a specific ppn:
        if (location.valid){ // The vpn is already mapped to some ppn, hence 'pte_ptr' is the last node.
            *(location.pte_ptr) = ((ppn << 12) | 1); // adding 12 zeros to the right and changing valid bot to 1
        }
        else{ //The vpn is not mapped, so 'pte_ptr' is some inner node.
            map_rest_of_path(vpn, ppn, location);
        }
    }
}
