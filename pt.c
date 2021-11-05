//
// Created by gail1 on 30-Oct-21.
//
#include <stdio.h> // todo delete
#include "os.h"

typedef struct{
    uint64_t *pte_ptr;
    int level;
    int valid;
} pte_loc;

pte_loc find(uint64_t pt, uint64_t vpn){
    printf("\n\t\t\tEntered function 'find':\n"); //todo delete
    pte_loc location;
    uint64_t* first_node = (int64_t*)phys_to_virt(pt << 12);
    uint64_t* curr_node = first_node;
    uint64_t curr_symbol;
    uint64_t curr_pte;
    uint64_t valid_bit;
    for (int i = 0; i<5; i++){
        printf("\n\t\t\tNode no. : %d\n", i); //todo delete
        curr_symbol = (vpn << (19+9*i)) >> 55;
        printf("\n\t\t\tcurrent symbol is: %d\n", curr_symbol); //todo delete
        curr_pte = curr_node[curr_symbol];
        valid_bit = curr_pte & 1;
        location.level = i;
        location.pte_ptr = &curr_pte;
        if (valid_bit == 0){
            printf("\n\t\t\tvalid bit is 0\n"); //todo delete
            location.valid = 0;
            break;
        }
        else{ // if valid_bit is 1:
            printf("\n\t\t\tvalid bit is 1\n"); //todo delete
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
    printf("\n\t\t\tEntered function 'map_rest_of_path':\n"); //todo delete
    uint64_t* curr_pte_ptr = location.pte_ptr;
    printf("\n\t\t\tcurrent pte address is: %p\n", curr_pte_ptr); // todo delete
    uint64_t* next_node_ptr;
    uint64_t next_symbol;
    for (int i = location.level; i<4;  i++){ // updating all the inner nodes
        printf("\n\t\t\tNode no. : %d\n", i); //todo delete
        uint64_t next_node_ppn = alloc_page_frame();
        uint64_t phys_addr = (next_node_ppn << 12); // adding 12 zeros to the right
        *curr_pte_ptr = (phys_addr | 1); // updating the current pte (in node i) and changing valid bit to 1;
        printf("\n\t\t\t valid bit in *curr_pte_ptr is: %d\n", (*curr_pte_ptr) & 1); // todo delete
        next_node_ptr = phys_to_virt(phys_addr); // pointer to the next node
        next_symbol = ((vpn << (19+9*(i+1))) >> 55);
        printf("\n\t\t\tnext symbol is: %d\n", next_symbol); //todo delete
        curr_pte_ptr = &(next_node_ptr[next_symbol]); // which is actually the "next pte pointer" not the current
    }
    *curr_pte_ptr = ((ppn << 12) | 1); // updating the leave (node number 4)
    printf("\n\t\t\tUpdated pte on the last node to %x\n", ppn); // todo delete
}

uint64_t page_table_query(uint64_t pt, uint64_t vpn){
    pte_loc location = find(pt, vpn);
    if (location.valid == 1){
        uint64_t ppn = ((*(location.pte_ptr)) >> 12);
        printf("\n\t\t\tgot to the last node in 'find' and returned ppn:  %x\n", ppn); // todo delete
        return ppn;
    }
    return NO_MAPPING;
}

void page_table_update(uint64_t pt, uint64_t vpn, uint64_t ppn){
    printf("\n\t\tEntering 'find' function:\n"); //todo delete
    pte_loc location = find(pt, vpn);
    if (ppn == NO_MAPPING){ // User asked to erase mapping:
        if (location.valid == 0){
            printf("\n\t\t the vpn already not valid, 'update function' did nothing.\n"); // todo delete
            return;
        } else{
            printf("\n\t\t changed valid bit to 0.\n");
            *(location.pte_ptr) = ((*(location.pte_ptr)) & (~1)); //changing valid bit to 0 (in the last node)
        }
    }
    else{ // User asked to map to a specific ppn:
        if (location.valid){ // The vpn is already mapped to some ppn, hence 'pte_ptr' is the last node.
            *(location.pte_ptr) = ((ppn << 12) | 1); // adding 12 zeros to the right and changing valid bot to 1
        }
        else{ //The vpn is not mapped, so 'pte_ptr' is some inner node.
            printf("\n\t\tvpn not mapped so we will build the rest of the path:\n"); //todo delete
            printf("\n\t\tlocation.pte_ptr is: %p\n", location.pte_ptr); // todo delete
            map_rest_of_path(vpn, ppn, location);
        }
    }
}
