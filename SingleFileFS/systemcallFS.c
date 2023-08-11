#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/buffer_head.h>

#include "SingleFileSystem.h"
void index_to_posizione(posizione * i,int index){
    i->sb= index / NUMEROMETADATABLOCK;
    i->offset = index - i->sb*NUMEROMETADATABLOCK;
}
int confronto_posizioni(posizione i,posizione j){
    return i.sb==j.sb && i.offset==j.offset;
}
void update_memory_struct(int index){
    posizione mypos;
    index_to_posizione(&mypos,index);
    
    struct buffer_head * bh_meta = sb_bread(the_sb,0);
    struct_sb_information * sb_meta = (struct_sb_information *) bh_meta->b_data;
    posizione last = sb_meta->last;
    posizione first = sb_meta->first;

    struct buffer_head * bh[2];
    struct_sb_metadata * metadata_block[5];

    bh[0]=sb_bread(the_sb,mypos.sb+1);
    metadata_block[0]=(struct_sb_metadata*) bh[0]->b_data;
    
    //valido il bit
    metadata_block[0]->vet[mypos.offset].valid=1;
    posizione prec = metadata_block[0]->vet[mypos.offset].prec;
    posizione succ = metadata_block[0]->vet[mypos.offset].succ;
    //in caso il file system sia nuovo
    if( last.offset == -1){
        printk("rete nuova\n");

        sb_meta->last.offset=mypos.offset;
        sb_meta->last.sb=mypos.sb;

        sb_meta->first.offset=mypos.offset;
        sb_meta->first.sb=mypos.sb;

        metadata_block[0]->vet[mypos.offset].prec.offset=-1;
        metadata_block[0]->vet[mypos.offset].prec.sb=-1;

        metadata_block[0]->vet[mypos.offset].succ.offset=-1;
        metadata_block[0]->vet[mypos.offset].succ.sb=-1;

        mark_buffer_dirty(bh[0]);
        mark_buffer_dirty(bh_meta);
        brelse(bh[0]);
        brelse(bh_meta);
        return;
    }
    //in caso sia già l'ultimo ad essere inserito
    if(confronto_posizioni(mypos,last)) {
        printk("l'indice é l'ultimo\n");
        mark_buffer_dirty(bh[0]);
        brelse(bh[0]);
        brelse(bh_meta);
        return;
    }
    //fase di sganciamento del nodo
    //in caso sia il primo a dover essere inserito
    if(confronto_posizioni(mypos,first)){
        printk("l'indice é il primo\n");
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
            brelse(bh[1]);
        }
        metadata_block[0]->vet[mypos.offset].succ.offset=-1;
    }
    if(!confronto_posizioni(succ,prec)){
        printk(" é in mezzo\n");
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
            brelse(bh[1]);
        }
        if(prec.sb == mypos.sb){
            metadata_block[0]->vet[prec.offset].prec.offset=succ.offset;
            metadata_block[0]->vet[prec.offset].prec.sb=succ.sb;
        }else{
            bh[1]=sb_bread(the_sb,prec.sb+1);
            metadata_block[1]=(struct_sb_metadata*) bh[1]->b_data;          
            metadata_block[1]->vet[succ.offset].prec.offset=succ.offset;
            metadata_block[1]->vet[succ.offset].prec.sb=succ.sb;
            mark_buffer_dirty(bh[1]);
            brelse(bh[1]);
        }
        
        metadata_block[0]->vet[mypos.offset].succ.offset=-1;
        metadata_block[0]->vet[mypos.offset].succ.sb=-1;
        metadata_block[0]->vet[mypos.offset].prec.offset=-1;
        metadata_block[0]->vet[mypos.offset].prec.sb=-1;
    }
    printk("Ho scollegato\n");
    if(mypos.sb == last.sb){
        metadata_block[0]->vet[last.offset].succ.offset=mypos.offset;
        metadata_block[0]->vet[last.offset].succ.sb=mypos.sb;
    }else{
            bh[1]=sb_bread(the_sb,last.sb+1);
            metadata_block[1]=(struct_sb_metadata*) bh[1]->b_data;          
            metadata_block[1]->vet[last.offset].succ.offset=mypos.offset;
            metadata_block[1]->vet[last.offset].succ.sb=mypos.sb;
            mark_buffer_dirty(bh[1]);
            brelse(bh[1]);
    }
    sb_meta->last.offset=mypos.offset;
    sb_meta->last.sb=mypos.sb;
    metadata_block[0]->vet[mypos.offset].succ.offset=-1;
    printk("Ho ricollegato\n");

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