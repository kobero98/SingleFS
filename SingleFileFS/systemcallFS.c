#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/buffer_head.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include "SingleFileSystem.h"
void index_to_posizione(posizione * i,int index){
    i->sb= index / DIMENSIONEVETTORE;
    i->offset = index - i->sb*DIMENSIONEVETTORE;
}
int confronto_posizioni(posizione i,posizione j){
   return i.sb==j.sb && i.offset==j.offset;
}
void update_memory_struct(int index){
    posizione mypos,last,first,prec,succ;
    struct buffer_head * bh_meta;
    struct buffer_head * bh[2];
    struct_sb_metadata * metadata_block[5];
    struct_sb_information * sb_meta;

    index_to_posizione(&mypos,index);
    bh_meta = sb_bread(the_sb,0);
    sb_meta = (struct_sb_information *) bh_meta->b_data;
    last = sb_meta->last;
    first = sb_meta->first;

    bh[0]=sb_bread(the_sb,mypos.sb+1);
    metadata_block[0]=(struct_sb_metadata*) bh[0]->b_data;
    
    //valido il bit
    metadata_block[0]->vet[mypos.offset].valid=1;
    prec = metadata_block[0]->vet[mypos.offset].prec;
    succ = metadata_block[0]->vet[mypos.offset].succ;
    //in caso il file system sia nuovo
    if( last.offset == -1){
        sb_meta->last.offset=mypos.offset;
        sb_meta->last.sb=mypos.sb;

        sb_meta->first.offset=mypos.offset;
        sb_meta->first.sb=mypos.sb;

        metadata_block[0]->vet[mypos.offset].prec.offset=-1;
        metadata_block[0]->vet[mypos.offset].prec.sb=-1;

        metadata_block[0]->vet[mypos.offset].succ.offset=-1;
        metadata_block[0]->vet[mypos.offset].succ.sb=-1;

        mark_buffer_dirty(bh[0]);
        SYNCRONUS sync_dirty_buffer(bh[0]);
        mark_buffer_dirty(bh_meta);
        SYNCRONUS sync_dirty_buffer(bh_meta);
        brelse(bh[0]);
        brelse(bh_meta);
        return;
    }
    //in caso sia ("già l'ultimo ad essere inserito")
    if(confronto_posizioni(mypos,last)) {
        mark_buffer_dirty(bh[0]);
        SYNCRONUS sync_dirty_buffer(bh[0]);
        brelse(bh[0]);
        brelse(bh_meta);
        return;
    }
    //fase di sganciamento del nodo
    //in caso sia il primo a dover essere inserito
    if(confronto_posizioni(mypos,first)){
        sb_meta->first.sb = succ.sb;
        sb_meta->first.offset = succ.offset;
        //modifico il successore
        if(succ.sb == mypos.sb){
            metadata_block[0]->vet[succ.offset].prec.offset=-1;
            metadata_block[0]->vet[succ.offset].prec.sb=-1;
        }else{
            bh[1]=sb_bread(the_sb,succ.sb+1);
            metadata_block[1]=(struct_sb_metadata*) bh[1]->b_data;          
            metadata_block[1]->vet[succ.offset].prec.offset=-1;
            metadata_block[1]->vet[succ.offset].prec.sb=-1;
            mark_buffer_dirty(bh[1]);
            SYNCRONUS sync_dirty_buffer(bh[1]);
            brelse(bh[1]);
        }
        metadata_block[0]->vet[mypos.offset].succ.offset=-1;
    }

    if(confronto_posizioni(succ,prec) != 1){
        //modifico il successore
        if(mypos.sb == succ.sb){
            metadata_block[0]->vet[succ.offset].prec.sb=prec.sb;
            metadata_block[0]->vet[succ.offset].prec.offset=prec.offset;
        }else{
            bh[1]=sb_bread(the_sb,succ.sb+1);
            metadata_block[1]=(struct_sb_metadata*) bh[1]->b_data;          
            metadata_block[1]->vet[succ.offset].prec.offset=prec.offset;
            metadata_block[1]->vet[succ.offset].prec.sb=prec.sb;
            mark_buffer_dirty(bh[1]);
            SYNCRONUS sync_dirty_buffer(bh[1]);
            brelse(bh[1]);
        }
        if(prec.sb == mypos.sb){
            metadata_block[0]->vet[prec.offset].succ.offset=succ.offset;
            metadata_block[0]->vet[prec.offset].succ.sb=succ.sb;
        }else{
            bh[1]=sb_bread(the_sb,prec.sb+1);
            metadata_block[1]=(struct_sb_metadata*) bh[1]->b_data;          
            metadata_block[1]->vet[prec.offset].succ.offset=succ.offset;
            metadata_block[1]->vet[prec.offset].succ.sb=succ.sb;
            mark_buffer_dirty(bh[1]);
            SYNCRONUS sync_dirty_buffer(bh[1]);
            brelse(bh[1]);
        }
        metadata_block[0]->vet[mypos.offset].succ.offset=-1;
        metadata_block[0]->vet[mypos.offset].succ.sb=-1;
        metadata_block[0]->vet[mypos.offset].prec.offset=-1;
        metadata_block[0]->vet[mypos.offset].prec.sb=-1;
    }

    if(mypos.sb == last.sb){
        metadata_block[0]->vet[last.offset].succ.offset=mypos.offset;
        metadata_block[0]->vet[last.offset].succ.sb=mypos.sb;
    }else{
            bh[1]=sb_bread(the_sb,last.sb+1);
            metadata_block[1]=(struct_sb_metadata*) bh[1]->b_data;          
            metadata_block[1]->vet[last.offset].succ.offset=mypos.offset;
            metadata_block[1]->vet[last.offset].succ.sb=mypos.sb;
            mark_buffer_dirty(bh[1]);
            SYNCRONUS sync_dirty_buffer(bh[1]);
            brelse(bh[1]);
    }
    sb_meta->last.offset=mypos.offset;
    sb_meta->last.sb=mypos.sb;
    metadata_block[0]->vet[mypos.offset].succ.offset=-1;

    mark_buffer_dirty(bh[0]);
    SYNCRONUS sync_dirty_buffer(bh[0]);
    mark_buffer_dirty(bh_meta);
    SYNCRONUS sync_dirty_buffer(bh_meta);
    brelse(bh[0]);
    brelse(bh_meta);
    return;
}
uint64_t getTime(void){
    uint64_t Uptime=0;
    uint64_t LowTime=0;
    uint64_t timetotal=0;
    __asm__ volatile (
    "rdtsc  \n"
    : "=a" (LowTime), "=d" (Uptime)
    :
    :  
    );
    timetotal = Uptime<<32 | LowTime;
    return timetotal;
}
int put_data(char * source,size_t size){
    bool compare;
    size_t ret;
    int index,diff,mask;
    struct buffer_head * bh;
    atomic_register *info;
    metadati_block_element * q;
    block_file_struct* block;
    //Dovrei effettuare lo scollegamento del blocco
    //forse non dovrei caricare tutto in memoria!
    q =(metadati_block_element *) kmalloc(sizeof(metadati_block_element),GFP_KERNEL);
    q->next=NULL;
    info =(atomic_register*) the_sb->s_fs_info;
    TakeTicket:    
    compare = __sync_bool_compare_and_swap(&(info->lockScrittore),0,1);
    if(compare == false){
        msleep(2);
        goto TakeTicket;
    }
    //inizio sezione con lock
    index = trovaBit();
    if(index==-1){
        AUDIT printk("modulefs-put: Non ci sono bit per la Put\n");
        kfree(q);
        __sync_fetch_and_sub(&(info->lockScrittore),1);
        return -ENOMEM;
    }
    q->block.time=getTime(); //Sotto lock mi permette di essere sicuro che due thread su core differenti prendano tempi differenti.
    q->block.index_block=index; //non é necessario farla in zona critica
    q->next=NULL;
    /* modifico il blocco di dati */
    //codice di sanitizzazione
    diff=MAXBLOCKDATA-size;
    mask=diff>>(sizeof(int)*8-1);
    size=((MAXBLOCKDATA) & (mask)) | ((size) & (~mask));
    bh=sb_bread(the_sb,index+NUMEROMETADATABLOCK+2);
    block=(block_file_struct*) bh->b_data;
    ret = copy_from_user(block->dati,source,size);
    block->size=size-ret;
    mark_buffer_dirty(bh);
    SYNCRONUS sync_dirty_buffer(bh);
    /* Modifico i metadati associati al file system*/
    update_memory_struct(index);
    //aggiungo in coda la struttura 
    if(unlikely(info->testa == NULL)) __atomic_store_n(&(info->testa),q,__ATOMIC_SEQ_CST);//accade solo una volta
    else __atomic_store_n(&(info->coda->next),q,__ATOMIC_SEQ_CST);
    __atomic_store_n(&(info->coda),q,__ATOMIC_SEQ_CST);
    //fine zona critica
    __sync_fetch_and_sub(&(info->lockScrittore),1);
    setBitUP(index);
    return index;
}
int get_data(int offset,char * destination,size_t size){
    size_t ret;
    struct buffer_head * bh;
    int diff,mask;
    block_file_struct* block;
    if(offset>=NBLOCK){
        AUDIT printk("modulefs-get: offset maggiore del numero massimo\n");
    }
    if(!checkBit(offset)){
        AUDIT printk("modulefs-get: offset non valido\n");
        return -ENODATA;
    }
    bh=sb_bread(the_sb,offset+NUMEROMETADATABLOCK+2);
    if(bh==NULL){
        //errore
        return -EIO;
    }
    block=(block_file_struct*) bh->b_data;
    diff = size - (block->size);
    mask=diff>>(sizeof(int)*8-1);
    size=(size & mask) | (block->size & ~mask);
    ret = copy_to_user(destination,block->dati,size);
    return size-ret;
}
int invalidate_data(int offset){
    posizione mypos;
    atomic_register *info;
    struct buffer_head * bh;
    struct_sb_metadata* metadati_block;
    registro_atomico *newReg;
    bool compare;
    metadati_block_element *q,*p;
    registro_atomico * oldReg;
    if(offset>=NBLOCK){
        AUDIT printk("modulefs-invalide: offset maggiore del numero massimo\n");
    }
    if(!checkBit(offset)){
        AUDIT printk("modulefs-invalide: offset non occuppato\n");
        return -ENODATA;
    }
    index_to_posizione(&mypos,offset);
    info =(atomic_register*) the_sb->s_fs_info;
    TakeTicketInvalid:    
    compare = __sync_bool_compare_and_swap(&(info->lockScrittore),0,1);
    if(compare == false){
        msleep(2);
        goto TakeTicketInvalid;
    }
    //da qui la get non può più vedere il dato aggiornato
    setBitDown(offset);
    //aggiorno i metadati dei superblocchi
    bh=sb_bread(the_sb,mypos.sb+1);
    metadati_block =(struct_sb_metadata*) bh->b_data;
    metadati_block->vet[mypos.offset].valid=0;
    mark_buffer_dirty(bh);
    SYNCRONUS sync_dirty_buffer(bh);
    brelse(bh);
    newReg =(registro_atomico *) kmalloc(sizeof(registro_atomico),GFP_KERNEL);
    newReg->num_entry=0;
    newReg->num_exit=0;
    p=NULL;
    q=info->testa;
    if(info->testa->block.index_block==offset){
        __sync_bool_compare_and_swap(&(info->coda),info->testa,p);
        __atomic_store_n(&(info->testa),info->testa->next,__ATOMIC_SEQ_CST);     
        oldReg=__atomic_exchange_n(&(info->reg),newReg,__ATOMIC_SEQ_CST);
        while(oldReg->num_entry != oldReg->num_exit)
        {
            msleep(1);//in caso che devo attendere rilascio la cpu
        };
        kfree(q);
        kfree(oldReg);
    }else{
        p=q;
        q=q->next;
        while(q!=NULL){ //metto secondo controllo
            if(q->block.index_block==offset){
                __sync_bool_compare_and_swap(&(info->coda),q,p);
                __sync_bool_compare_and_swap(&(p->next),q,q->next);
                oldReg=__atomic_exchange_n(&(info->reg),newReg,__ATOMIC_SEQ_CST);
                //aspetta che sono usciti tutti quelli che leggono
                //mi salvo quanti sono dentro potrebe essere un numero più grande di quello che realmente dovevo aspettare
                while(oldReg->num_entry != oldReg->num_exit){
                    msleep(1);//in caso che devo attendere rilascio la cpu
                };
                kfree(q);
                kfree(oldReg);
                break;
            }
            p=p->next;
            q=q->next;
        }
    }
    __sync_fetch_and_sub(&(info->lockScrittore),1);
    return 0;
}