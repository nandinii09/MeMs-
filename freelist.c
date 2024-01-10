#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>


struct main_node *main_head, *main_temp, *temp;
struct node* sub_temp;

struct node
{
    struct node *prev;
    struct node *next;
    size_t size;    // SIZE USER REQUESTS FOR
    bool process; // 0 -> hole, 1-> process

};

struct main_node
{
    struct main_node* next;
    struct node *sub_chain_head; //prev
    size_t size;                 // holds the size of entire subchain, is a multiple of PAGE_SIZE
};

struct node* split_sub_node(size_t bytes, struct node* original_node)
{
    struct node *new_sub_node;
    new_sub_node = (struct node *)mmap(NULL, original_node-> size - bytes, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (new_sub_node == MAP_FAILED){
        perror("mmap");
        exit(1);
    }

    new_sub_node->size = original_node-> size - bytes;
    new_sub_node->next = original_node-> next;
    new_sub_node->prev = original_node;
    new_sub_node->process=0;

    if(original_node->next != NULL){
        original_node->next->prev = new_sub_node;
    }
    original_node->next = new_sub_node;
    original_node->size = bytes;
    original_node->process = 1;

    return new_sub_node;
}

struct node* insertion_sub_node(struct main_node* head, size_t bytes, bool process)
{
    struct node *new_sub_node;
    new_sub_node = (struct node *)mmap(NULL, sizeof(struct node), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (new_sub_node == MAP_FAILED){
        perror("mmap");
        exit(1);
    }

    new_sub_node->size = bytes;
    new_sub_node->next = NULL;
    new_sub_node->prev = NULL;
    new_sub_node->process = process;

    if (head->sub_chain_head == NULL){
        head->sub_chain_head = new_sub_node;
    }
    else{
        sub_temp = head->sub_chain_head;
        while (sub_temp->next != NULL)
        {
            sub_temp = sub_temp->next;
        }
        sub_temp->next = new_sub_node;
        new_sub_node->prev = sub_temp;
    }

    //printf("\nsub node inserted %d\n", new_sub_node);
    return new_sub_node;
}

struct main_node* insert_main_node(size_t bytes, size_t mem_size_req) //bytes = page size
{
    struct main_node *new_main_node = (struct main_node *)mmap(NULL, sizeof(struct main_node), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (new_main_node == MAP_FAILED){
        perror("mmap");
        exit(1);
    }

    new_main_node->size = bytes;
    new_main_node->sub_chain_head = NULL;
    new_main_node->next = NULL;

    if (main_head == NULL){
        main_head = new_main_node;
    }
    else
    {
        temp = main_head;
        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = new_main_node;
        //new_main_node->prev = temp;
    }
    new_main_node -> sub_chain_head = insertion_sub_node(new_main_node, mem_size_req,true);
    insertion_sub_node(new_main_node, bytes - mem_size_req,false);
    //printf("\nmain node inserted %d\n", new_main_node);
    return new_main_node;
}
struct node* join_nodes(struct node* first_node, struct node* second_node){

    first_node->next = second_node->next;
    first_node->size += second_node->size;
    if (second_node->next != NULL) {
        second_node->next->prev = first_node;
    }
    munmap(second_node, sizeof(struct node));
    return first_node;
}

void display()
{
    if (main_head == NULL){
        printf("NO MEMORY ALLOCATED YET\n");
    }
    else
    {
        main_temp = main_head;

        while (main_temp != NULL)
        {
            printf("* %zu %d", main_temp->size, main_temp);
            if (main_temp->sub_chain_head == NULL){
                printf("SUB CHAIN EMPTY\n");
            }else
            {
                sub_temp = main_temp->sub_chain_head;                                     // 36913152, 37011456, 37109760

                while (sub_temp != NULL)
                {
                    printf("-> %zu (%d) (%d)", sub_temp->size, sub_temp->process, sub_temp);
                    sub_temp = sub_temp->next;
                }
                printf("\n");
            }
            main_temp = main_temp->next;
        }
    }
}

//int main1(){
//    printf("heree\n");
//    main_head = insert_main_node(1000);
//    temp = main_head;
//    insertion_sub_node(temp, 600, 0);
//    insertion_sub_node(temp, 400, 1);
//    display();
//    // temp = insert_main_node(600);
//    // insertion_sub_node(temp, 300, 0);
//    // struct node* node1 = insertion_sub_node(temp, 200, 1);
//    // struct node* node2 = insertion_sub_node(temp, 100, 2);
//    struct node* last = get_last_node(main_head);
//    printf("%ld\n", last->size);
//    // join_nodes(node1, node2);
//    return 0;
//}

