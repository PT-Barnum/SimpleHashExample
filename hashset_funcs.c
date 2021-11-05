
#include "hashset.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

long hashcode(char key[]){
  union {
    char str[8];
    long num;
  } strnum;
  strnum.num = 0;

  for(int i=0; i<8; i++){
    if(key[i] == '\0'){
      break;
    }
    strnum.str[i] = key[i];
  }
  return strnum.num;
}


void hashset_init(hashset_t *hs, int table_size) {
  hs->table = malloc(sizeof(hashnode_t*)*table_size);
  hs->table_size = table_size;
  hs->item_count = 0;
  for (int i=0;i<table_size;i++) {
    hs->table[i] = NULL;
  }
  hs->order_first = NULL;
  hs->order_last = NULL;
}


int hashset_contains(hashset_t *hs, char item[]) {
  int index = hashcode(item) % hs->table_size;
  if (index < 0) {
      index = -(index);
  }
  if (hs->table[index] == NULL) {
    return 0;
  }
  hashnode_t *comp = hs->table[index];
  if (strcmp(item,comp->item) == 0) {
    return 1;
  }
  else {
    if (comp->table_next != NULL) {
      comp = comp->table_next;
      while (comp != NULL) {
        if (strcmp(item, comp->item) == 0) {
          return 1;
        }
        comp = comp->table_next;
      }
    }
  }
return 0;
}


int hashset_add(hashset_t *hs, char item[]) {
  if (hashset_contains(hs,item) == 1) {
    return 0;
  }
  int index = hashcode(item) % hs->table_size;
  if (index < 0) {
    index = -(index);
  }
  hashnode_t *new = malloc(sizeof(hashnode_t));
  strcpy(new->item,item);

  if (hs->table[index] == NULL) {                            //this if conditional is based on the nodes at
    hs->table[index] = new;                                  //this specific index value in the table
    hs->table[index]->table_next = NULL;
    hs->table[index]->order_next = NULL;
  }
  else {
    new->table_next = hs->table[index];
    hs->table[index] = new;
    hs->table[index]->order_next = NULL;
  }

  if (hs->order_first == NULL) {                             //this if conditional is based on the nodes in
    hs->order_first = new;                                   //the overall ordering which they are added to
    hs->order_last = new;                                    //the hashset
  }
  else if (strcmp(hs->order_first->item,hs->order_last->item) == 0) {
    hs->order_last = new;
    hs->order_first->order_next = new;
  }
  else {
    hs->order_last->order_next = new;
    hs->order_last = new;
  }

  hs->item_count++;
  return 1;
}


void hashset_free_fields(hashset_t *hs) {
  hashnode_t *place = hs->order_first;
  while (place != NULL) {
    hashnode_t *freePlace = place;
    place = place->order_next;
    free(freePlace);
  }
  hs->item_count = 0;
  hs->order_first = NULL;
  hs->order_last = NULL;
  free(hs->table);
}


void hashset_show_structure(hashset_t *hs) {
  printf("item_count: %d\n", hs->item_count);
  printf("table_size: %d\n", hs->table_size);
  if (hs->order_first == NULL) {
    printf("order_first: NULL\n");
    printf("order_last : NULL\n");
  }
  else {
    printf("order_first: %s\n", hs->order_first->item);
    printf("order_last : %s\n", hs->order_last->item);
  }
  double loadFact = (double) hs->item_count / (double) hs->table_size;
  printf("load_factor: %.4f\n", loadFact);
  for (int i=0;i<hs->table_size;i++) {
    printf("[%2d] : ", i);
    if (hs->table[i] != NULL) {                                       //determines what to print for any given node
      hashnode_t *node = hs->table[i];                                //data based on whether the node connects to
      while (node != NULL) {                                          //another node using the "order" list
        if (node->order_next == NULL) {
          printf("{%ld %s >>NULL} ", hashcode(node->item), node->item);
        }
        else {
          printf("{%ld %s >>%s} ", hashcode(node->item), node->item, node->order_next->item);
        }
        node = node->table_next;
      }
      printf("\n");
    }
    else {
      printf("\n");
    }
  }
}


void hashset_write_items_ordered(hashset_t *hs, FILE *out) {
  hashnode_t *node = hs->order_first;
  if (node != NULL) {
    int count = 1;
    fprintf(out,"  %d %s\n", count, node->item);
    if (strcmp(hs->order_first->item,hs->order_last->item) != 0) {
      while (node->order_next != NULL) {
        node = node->order_next;
        count++;
        fprintf(out,"  %d %s\n", count, node->item);
      }
    }
  }
}


void hashset_save(hashset_t *hs, char *filename) {
  FILE *file = fopen(filename,"w");
  if (file == NULL) {
    printf("ERROR: could not open file '%s'\n",filename);
  }
  else {
    fprintf(file, "%d %d\n", hs->table_size, hs->item_count);
    hashset_write_items_ordered(hs,file);
    fclose(file);
  }
}


int hashset_load(hashset_t *hs, char *filename) {
  FILE *filePoint = fopen(filename,"r");
  if (filePoint == NULL) {
    printf("ERROR: could not open file '%s'\n", filename);
    return 0;
  }
  else {
    hashset_free_fields(hs);
    int newTable;
    int newItemNum;
    int lineCheck = fscanf(filePoint, "%d %d", &newTable, &newItemNum);
    hashset_init(hs,newTable);
    char *item = malloc(128);                                        //128 is the maximum size for hs->table[i]->item
    int countOrder;                                                  //the allocated memory for "item" is free'd at the
    lineCheck = fscanf(filePoint,"%d %s", &countOrder, item);        //end of the function call
    while (lineCheck != EOF) {
      hashset_add(hs,item);
      lineCheck = fscanf(filePoint,"%d %s", &countOrder, item);
    }
    free(item);
  }
  fclose(filePoint);
  return 1;
}


int next_prime(int num) {
  int primeCheck = 2;
  while (1) {                                               //uses a while loop with statement "1" because
    if (num % primeCheck == 0) {                            //the terminating criteria is reached when the
      num++;                                                //lower boundary of values divisible within num
      primeCheck = 2;                                       //becomes greater than num/2, meaning num is not
    }                                                       //divisible by any other value
    else if (primeCheck > num/2) {
      break;
    }
    else {
      primeCheck++;
    }
  }
  return num;
}


void hashset_expand(hashset_t *hs) {
  int newTable = next_prime((2*hs->table_size)+1);
  hashset_t *tempHash = malloc(sizeof(hashset_t));          //initializes a second hashset to take in
  hashset_init(tempHash,hs->table_size);                    //all the node information from the original
  hashnode_t *node = hs->order_first;                       //hashset. Then copies it back over when the
  while (node != NULL) {                                    //new table length is calculated using the
    hashset_add(tempHash,node->item);                       //next_prime function
    node = node->order_next;
  }
  hashset_free_fields(hs);
  hashset_init(hs,newTable);
  node = tempHash->order_first;
  while (node != NULL) {
    hashset_add(hs,node->item);
    node = node->order_next;
  }
  hashset_free_fields(tempHash);
  free(tempHash);
}
