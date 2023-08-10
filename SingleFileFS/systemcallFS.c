#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/buffer_head.h>

#include "SingleFileSystem.h"

void update_memory_struct(int index){
    int index_mysb = index/NUMEROMETADATABLOCK;
    struct buffer_head * bh_meta = sb_bread(the_sb,0);
    struct_sb_information * mysb = (struct_sb_information *) bh_meta->b_data;
    posizione last = mysb->last;
    posizione first = mysb->first;
    int index_sb_last = last.index / NUMEROMETADATABLOCK;
    struct buffer_head * bh[2];
    struct_sb_metadata * metadata_block[5];

    bh[0]=sb_bread(the_sb,mysb+1);
    metadata_block[0]=(struct_sb_metadata*) bh[0]->b_data;

    //valido il bit
    metadata_block[0]->vet[index%NUMEROMETADATABLOCK].valid=1;
    posizione prec=metadata_block[0]->vet[index%NUMEROMETADATABLOCK].prec;
    posizione succ=metadata_block[0]->vet[index%NUMEROMETADATABLOCK].succ;
    int index_sb_succ=prec.index/NUMEROMETADATABLOCK;
    int index_sb_prec=succ.index/NUMEROMETADATABLOCK;
    //in caso il file system sia nuovo
    if( last.index == -1){
        printk("rete nuova\n");
        mysb->last.index=index;
        mysb->first.index=index;
        metadata_block[0]->vet[index % NUMEROMETADATABLOCK].prec.index=-1;
        metadata_block[0]->vet[index % NUMEROMETADATABLOCK].succ.index=-1;
        mark_buffer_dirty(bh[0]);
        mark_buffer_dirty(bh_meta);
        brelse(bh[0]);
        brelse(bh_meta);
        return;
    }
    //in caso sia già l'ultimo ad essere inserito
    if(index == last.index) {
        printk("l'indice é l'ultimo\n");
        mark_buffer_dirty(bh[0]);
        brelse(bh[0]);
        brelse(bh_meta);
        return;
    }
    //fase di sganciamento del nodo
    //in caso sia il primo a dover essere inserito
    if(index == first.index){
        printk("l'indice é il primo\n");
        mysb->first.index = succ.index;
        //modifico il successore
        if(index_sb_succ == index_mysb){
            metadata_block[0]->vet[succ.index % NUMEROMETADATABLOCK].prec.index=-1;
        }else{
            bh[1]=sb_bread(the_sb,index_sb_succ+1);
            metadata_block[1]=(struct_sb_metadata*) bh[1]->b_data;          
            metadata_block[1]->vet[succ.index % NUMEROMETADATABLOCK].prec.index=-1;
            mark_buffer_dirty(bh[1]);
            brelse(bh[1]);
        }
        metadata_block[0]->vet[index % NUMEROMETADATABLOCK].succ.index=-1;
    }
    if(index_sb_succ!=index_sb_prec){
        printk(" é in mezzo\n");
        //modifico il successore
        if(index_sb_succ == index_mysb){
            metadata_block[0]->vet[succ.index % NUMEROMETADATABLOCK].prec.index=prec.index;
        }else{
            bh[1]=sb_bread(the_sb,index_sb_succ+1);
            metadata_block[1]=(struct_sb_metadata*) bh[1]->b_data;          
            metadata_block[1]->vet[succ.index % NUMEROMETADATABLOCK].prec.index=-1;
            mark_buffer_dirty(bh[1]);
            brelse(bh[1]);
        }
        if(index_sb_prec == index_mysb){
            metadata_block[0]->vet[prec.index % NUMEROMETADATABLOCK].prec.index=succ.index;
        }else{
            bh[1]=sb_bread(the_sb,index_sb_prec+1);
            metadata_block[1]=(struct_sb_metadata*) bh[1]->b_data;          
            metadata_block[1]->vet[succ.index % NUMEROMETADATABLOCK].prec.index=succ.index;
            mark_buffer_dirty(bh[1]);
            brelse(bh[1]);
        }
        metadata_block[0]->vet[index % NUMEROMETADATABLOCK].succ.index=-1;
        metadata_block[0]->vet[index % NUMEROMETADATABLOCK].prec.index=-1;
    }
    printk("Ho scollegato\n");
    if(index_mysb == index_sb_last){
        metadata_block[0]->vet[last.index % NUMEROMETADATABLOCK].succ.index=index;
    }else{
            bh[1]=sb_bread(the_sb,index_sb_last+1);
            metadata_block[1]=(struct_sb_metadata*) bh[1]->b_data;          
            metadata_block[1]->vet[last.index % NUMEROMETADATABLOCK].succ.index=index;
            mark_buffer_dirty(bh[1]);
            brelse(bh[1]);
    }
    mysb->last.index=index;
    metadata_block[0]->vet[index % NUMEROMETADATABLOCK].succ.index=-1;
    mark_buffer_dirty(bh[0]);
    mark_buffer_dirty(bh_meta);
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
    printk("sono la systemcall put_data\n");
    return 0;
    if(size>MAXBLOCKDATA){
        return -1;
    }
    //Dovrei effettuare lo scollegamento del blocco
    //forse non dovrei caricare tutto in memoria!
    metadati_block_element * q= kmalloc(sizeof(metadati_block_element),GFP_KERNEL);
    q->next=NULL;
    atomic_register *info =(atomic_register*) the_sb->s_fs_info;
TakeTicket:    
        compare = __sync_bool_compare_and_swap(&(info->lockScrittore),0,1);
        if(compare == false){
            msleep(2);
            goto TakeTicket;
        }
        //inizio sezione con lock
        int index = trovaBit();
        if(index==-1){
            __sync_fetch_and_sub(&(info->lockScrittore),1);
            printk("non ci sono blocchi disponibili\n");
            return -1;
        }
        q->block.time=getTime(); //Sotto lock mi permette di essere sicuro che due thread su core differenti prendano tempi differenti.
        q->block.index_block=index; //non é necessario farla in zona critica
        q->next=NULL;
        info->coda=q;
        /* Modifico i metadati associati al file system*/
        update_memory_struct(index);
        //fine zona critica
        __sync_fetch_and_sub(&(info->lockScrittore),1);
        /* modifico il blocco di dati */
        size=size & (MAXBLOCKDATA);
        struct buffer_head * bh=sb_bread(the_sb,index+NUMEROMETADATABLOCK+2);
        block_file_struct* block=(block_file_struct*) bh->b_data;
        block->block_information.time=q->block.time;
        ret = copy_from_user(block->dati,source,size);
        block->size=size-ret;
        mark_buffer_dirty(bh);
        //TO-DO:aggiungere la SYNC
        //completo l'operazione quando la bitmask viene aggiornata!, facendolo in modo atomico non serve il mutex
        //__sync_fetch_and_or(&(info->bitmask),(1 << index));
        setBit(index);
    return index;
}
int get_data(int offset,char * destination,size_t size){
    //controllare l'offset > 0 e <NBLOCK
    if(offset<0 || offset>=NBLOCK){
        return -1;
    }
    printk("get\n");
    bool compare;
    size_t ret;
    atomic_register *info =(atomic_register*) the_sb->s_fs_info;
    __sync_fetch_and_add(&(info->atomic_entry),1);
    if(!checkBit(offset)){
        printk("checkBit not pass\n");
        printBitMask();
        //non so se questo controllo potrebbe dare falsi negativi(?)
        __sync_fetch_and_add(&(info->atomic_exit),1);
        return -1;
    }
    //controllo che nessuno voglia scrivere
    //(potremmo consentire scirture?si ma non invalidazioni)
    scrittore:    
        compare=__sync_bool_compare_and_swap(&(info->lockScrittore),0,info->lockScrittore);
        if(compare==false){
            msleep(2);
            goto scrittore;
        }
        struct buffer_head * bh=sb_bread(the_sb,offset+NUMEROMETADATABLOCK+2);
        if(bh==NULL){
            //errore
            __sync_fetch_and_add(&(info->atomic_exit),1);
            printk("bufferHead not pass\n");
            return -1;
        }
        block_file_struct* block=(block_file_struct*) bh->b_data;
        size = size & (block->size);
        printk("size=%d",size);
        ret = copy_to_user(destination,block->dati,size);
    return size-ret;
}
int invalidate_data(int offset){
    printk("sono la systemcall inv_data\n");
    return 0;
}