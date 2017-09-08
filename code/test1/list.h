struct list_head{
    struct list_head *next, *prev;
}

// #define LIST_HEAD_INIT(name) { &(name), &(name) }
// #define LIST_HEAD(name) \
//     struct list_head name = LIST_HEAD_INIT(name)

/* include/linux/list.h */
static inline void INIT_LIST_HEAD(struct list_head *list)
{
  /* 将前驱和后继节点都指向本身 */
  list->next = list;
  list->prev = list;
}