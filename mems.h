#include<stdio.h>
#include<stdlib.h>
// #import "freelist.c"
#include "freelist.c"
#define PAGE_SIZE 4096
#define starting_va 1000;

void* virtual_add;

void mems_init(){
    struct main_node *main_head = NULL;
    virtual_add= 1000;

}

void mems_finish() {
    main_temp = main_head;
    while (main_temp != NULL) {
        sub_temp = main_temp->sub_chain_head;
        while (sub_temp != NULL) {
            struct node* temp = sub_temp;
            sub_temp = sub_temp->next;
            munmap(temp, sizeof(struct node));
        }
        struct main_node* main_temp_node = main_temp;
        main_temp = main_temp->next;
        munmap(main_temp_node, sizeof(struct main_node));
    }
    main_head = NULL;
    printf("Memory deallocated ");
}

void* mems_malloc(size_t mem_size_req){
    bool found_segment = false;
    if (main_head == NULL){
        size_t page =  (mem_size_req / PAGE_SIZE + 1) * PAGE_SIZE;
        main_head = insert_main_node(page, mem_size_req);
        void* temp = virtual_add;
        virtual_add += page;
//        virtual_add += mem_size_req;
        //printf("Physical address: %d ", main_head->sub_chain_head);
        return temp;
    } else {
        main_temp = main_head;
        void* temp_add = (void*) starting_va;
        while (main_temp != NULL && !found_segment ){                 // hole is found
            if (main_temp->sub_chain_head != NULL){
                sub_temp = main_temp->sub_chain_head;
                while (sub_temp != NULL && !found_segment )
                {
                    if(sub_temp->process == 0){       // hole condition.
                        if ( mem_size_req == sub_temp->size){
                            sub_temp->process =1;
                            found_segment = true;
//                            printf("Physical address: %lu \n",(unsigned long) sub_temp);
                            return temp_add;
                        } else if (mem_size_req < sub_temp->size){
                            split_sub_node(mem_size_req,sub_temp);
                            found_segment = true;
//                            printf("Physical address: %lu \n", sub_temp);
                            return temp_add;
                        }
                    }
                    temp_add += sub_temp->size;
                    sub_temp = sub_temp->next;
                }
            }
            main_temp = main_temp->next;
            }
    }
    if (!found_segment){
        size_t page =  (mem_size_req / PAGE_SIZE) + 1 * PAGE_SIZE;
        struct main_node* new_m_node = insert_main_node(page, mem_size_req);
        void* temp = virtual_add;
        virtual_add += page;
        //printf("%d %d", main_head, main_head->sub_chain_head);
        //printf("Physical address: %d \n", new_m_node->sub_chain_head);
        return temp;
    }
}
void mems_print_stats() {
    printf("\n");
    if (main_head == NULL) {
        printf("NO MEMORY ALLOCATED YET\n");
        return;
    }

    main_temp = main_head;
    size_t temp_va = starting_va;

    int page_count = 0;
    size_t space_unused = 0;
    int main_chain_len = 0;
    int sub_chain_len;
    int sub_chain_array[100]; // Make sure this array is large enough for your use case.

    while (main_temp != NULL) {
        main_chain_len++;
        printf("MAIN[ %d : %d ] ", temp_va, (temp_va + main_temp->size - 1));

        page_count += main_temp->size / 4096;

        if (main_temp->sub_chain_head == NULL) {
            printf("SUB CHAIN EMPTY\n");
        } else {
            sub_temp = main_temp->sub_chain_head;
            sub_chain_len = 0;
            while (sub_temp != NULL) {
                sub_chain_len++;

                if (sub_temp->process) {
                    printf("<-> P[ %d : %d ] ", temp_va, (temp_va + sub_temp->size - 1));
                    temp_va += sub_temp->size;
                } else {
                    printf("<-> H[ %d : %d ] ", temp_va, (temp_va + sub_temp->size - 1));
                    temp_va += sub_temp->size;
                    space_unused += sub_temp->size;
                    //printf("space unused: %d", space_unused);
                }

                sub_temp = sub_temp->next;
            }

            sub_chain_array[main_chain_len - 1] = sub_chain_len;
            printf("<-> NULL\n");
        }

        main_temp = main_temp->next;
    }

    printf("\nPages used: %d\n", page_count);
    printf("Space unused: %zu\n", space_unused);
    printf("Main chain length: %d\n", main_chain_len);

    printf("Sub-chain length array: [");
    for (int i = 0; i < main_chain_len; i++) {
        printf("%d", sub_chain_array[i]);
        if (i < main_chain_len - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

/*
Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).
*/
void* mems_get(void*v_ptr){
    //printf("\n%d va needed\n", v_ptr);
    if (main_head == NULL){
        printf("NO MEMORY ALLOCATED YET\n");
        return main_head;
    }
    else
    {
        size_t temp_size = (size_t) starting_va;
        main_temp = main_head;
        while (main_temp != NULL){
            //printf("here\n");
            //printf("%\n", (size_t) v_ptr >= temp_size + sub_temp->size );
            if ((main_temp->size +  temp_size) <= (size_t) v_ptr){
                //printf("skipped\n");
                temp_size+= main_temp->size;
                main_temp = main_temp->next;
            }else{
                sub_temp = main_temp->sub_chain_head;
                while (sub_temp != NULL && (size_t) v_ptr >= temp_size + sub_temp->size ) {
                    if (sub_temp->size != NULL) {
                        temp_size += sub_temp->size;
                        sub_temp = sub_temp->next;
                    }
                }  //printf("VA NEEDED: %d --%d %d\n", v_ptr, temp_size, sub_temp->size); //not checked
                if(sub_temp->size== (size_t) v_ptr || temp_size == (size_t) v_ptr){             // checked
                    //printf("VA NEEDED: %d ------1000-------- PHYSICAL ADD: %lu \n", v_ptr, (unsigned long) sub_temp); //not checked
                    return sub_temp;
                }
                else{
                    main_temp = main_temp->next;//checked
                    //printf("--------------------CHECK HO GAYAAAA-------------------\n");
                    //printf("VA NEEDED: %d -------------- PHYSICAL ADD: %lu", v_ptr, (unsigned long)  (size_t) sub_temp->prev  + (v_ptr -sub_temp->prev->size) ); //1004
                    return  (size_t) sub_temp->prev  + (v_ptr -sub_temp->prev->size);
                }
            }

        }
    }
}

/*
this function free up the memory pointed by our virtual_address and add it to the free list
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: nothing
*/

void mems_free(void *v_ptr){            //
    //printf("\n%d va needed\n", v_ptr);
    if (main_head == NULL){
        printf("NO MEMORY ALLOCATED YET\n");
        return;
    }
    else
    {
        size_t temp_size = (size_t) starting_va;
        main_temp = main_head;
        while (main_temp != NULL){
            if ((main_temp->size +  temp_size) <= (size_t) v_ptr){
                //printf("skipped\n");
                temp_size+= main_temp->size;
                main_temp = main_temp->next;
            }else{
                sub_temp = main_temp->sub_chain_head;
                while (sub_temp != NULL && (size_t) v_ptr >= temp_size + sub_temp->size ) {
                        temp_size += sub_temp->size;
                        sub_temp = sub_temp->next;
                }  //printf("VA NEEDED: %d --%d %d\n", v_ptr, temp_size, sub_temp->size); //not checked
                if (sub_temp->process ==0){
                    printf("\nMemory block already free.\n");
                    break;
                }
                if (temp_size == (size_t) v_ptr) {
                    if (sub_temp->process == 1) {
                        sub_temp->process = 0;
                        if (main_temp->sub_chain_head != sub_temp && sub_temp->prev->process == 0) {
                            join_nodes(sub_temp->prev, sub_temp);
                            return;
                        } else if (sub_temp->next != NULL && sub_temp->next->process == 0) {    // checked
                            join_nodes(sub_temp, sub_temp->next);
                            return;
                        }
                    }
                }
            }

        }
    }
}


