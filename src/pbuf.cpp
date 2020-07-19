//
// Created by christianb on 18.07.20.
//

#include <lwip/pbuf.h>

extern "C" {

#define PBUF_MAX_POOL_SIZE 1520

struct pbuf *pbuf_alloc(pbuf_layer l, u16_t length, pbuf_type type){

    struct pbuf * newBuffer= new struct pbuf;
    switch(type){
        case PBUF_POOL: {
            newBuffer->len = PBUF_MAX_POOL_SIZE;
            newBuffer->next = nullptr;
            newBuffer->payload = new uint8_t[PBUF_MAX_POOL_SIZE];
            newBuffer->ref = 1;
            newBuffer->tot_len = length;
            newBuffer->flags = 0;
            newBuffer->type_internal = PBUF_POOL;

            auto lastBuffer = newBuffer;
            for (uint32_t i = PBUF_MAX_POOL_SIZE; i < length; i += PBUF_MAX_POOL_SIZE) {
                lastBuffer->next = new struct pbuf;
                lastBuffer = lastBuffer->next;
                lastBuffer->len = PBUF_MAX_POOL_SIZE;
                lastBuffer->next = nullptr;
                lastBuffer->payload = new uint8_t[PBUF_MAX_POOL_SIZE];
                lastBuffer->ref = 1;
                lastBuffer->tot_len = length;
                lastBuffer->flags = 0;
                lastBuffer->type_internal = PBUF_POOL;
            }
            lastBuffer->len = length % PBUF_MAX_POOL_SIZE;
            if (lastBuffer->len == 0) {
                lastBuffer->len = PBUF_MAX_POOL_SIZE;
            }
        }
            break;
        case PBUF_RAM:
            newBuffer->len=length;
            newBuffer->next=nullptr;
            newBuffer->payload=new uint8_t[length];
            newBuffer->ref=1;
            newBuffer->tot_len=length;
            newBuffer->flags=0;
            newBuffer->type_internal=PBUF_RAM;
            break;
        case PBUF_REF:
            newBuffer->len=length;
            newBuffer->next=nullptr;
            newBuffer->payload=nullptr;
            newBuffer->ref=1;
            newBuffer->tot_len=length;
            newBuffer->flags=0;
            newBuffer->type_internal=PBUF_REF;
            break;
        case PBUF_ROM:
            newBuffer->len=length;
            newBuffer->next=nullptr;
            newBuffer->payload=nullptr;
            newBuffer->ref=1;
            newBuffer->tot_len=length;
            newBuffer->flags=0;
            newBuffer->type_internal=PBUF_ROM;
            break;
    }
    return newBuffer;
}

u8_t pbuf_free(struct pbuf *p) {
    if(p== nullptr) {
        return ERR_OK;
    }
    if(p->ref>0){
        p->ref--;
    }
    if(p->ref==0){
        if(p->type_internal&PBUF_TYPE_ALLOC_SRC_MASK_STD_MEMP_PBUF_POOL==PBUF_TYPE_ALLOC_SRC_MASK_STD_MEMP_PBUF_POOL){
            delete p->payload;
        }
        pbuf_free(p->next);
    }
    delete p;
    return ERR_OK;
}

}