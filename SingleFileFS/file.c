#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "SingleFileSystem.h"


ssize_t onefilefs_read(struct file * filp, char __user * buf, size_t len, loff_t * off) {

    printk("sto leggendo i dati\n");
    struct buffer_head *bh = NULL;
    struct inode * the_inode = filp->f_inode;
    uint64_t file_size = the_inode->i_size;
    int ret;
    loff_t offset;
    int block_to_read;//index of the block to be read from device

    printk("%s: read operation called with len %ld - and offset %lld (the current file size is %lld)",MOD_NAME, len, *off, file_size);

    //this operation is not synchronized 
    //*off can be changed concurrently 
    //add synchronization if you need it for any reason

    //check that *off is within boundaries
    if (*off >= file_size)
        return 0;
    else if (*off + len > file_size)
        len = file_size - *off;

    //determine the block level offset for the operation
    offset = *off % DEFAULT_BLOCK_SIZE; 
    //just read stuff in a single block - residuals will be managed at the applicatin level
    if (offset + len > DEFAULT_BLOCK_SIZE)
        len = DEFAULT_BLOCK_SIZE - offset;

    //compute the actual index of the the block to be read from device
    block_to_read = *off / DEFAULT_BLOCK_SIZE + 2; //the value 2 accounts for superblock and file-inode on device
 
    printk("%s: read operation must access block %d of the device",MOD_NAME, block_to_read);

    bh = (struct buffer_head *)sb_bread(filp->f_path.dentry->d_inode->i_sb, block_to_read);
    if(!bh){
	return -EIO;
    }
    ret = copy_to_user(buf,bh->b_data + offset, len);
    *off += (len - ret);
    brelse(bh);
    return len - ret;

}

ssize_t myfileops_read(struct file * filp, char __user * buf, size_t len, loff_t * off){  
    int i=0;
    int ret=0;
    myfiledata_struct* s = (myfiledata_struct*)filp->private_data;
    int time = s->num;
    int maxTime =  time;
    char * string_to_pass = kmalloc(len,0);//GFP_KERNEL
    struct super_block * sb = filp->f_inode->i_sb;
    if(testa_valid == NULL) printk("errore\n");
    for(;i<NBLOCK;i++){
        if(ret<len){
            __sync_fetch_and_add(&(testa_valid->countLettore),1);
            if(testa_valid->valid==1){
                printk("e qui?%d\n",testa_valid->time);
                if(testa_valid->time > time){
                    printk("e qui?%d\n",testa_valid->dimension);
                    if(ret+testa_valid->dimension<=len){
                        struct buffer_head *bh = sb_bread(sb,testa_valid->index_block);
                        printk("string to create %s",((block_file_struct*)bh->b_data)->dati);
                        strncpy(string_to_pass+ret,((block_file_struct*)bh->b_data)->dati,testa_valid->dimension);
                        //int ret1 = copy_to_user(buf,((block_file_struct*) bh->b_data)->dati,metadata_vector[i].dimension);
                        printk("string to pass: %s\n",string_to_pass); 
                        ret = ret + testa_valid->dimension;
                        if(testa_valid->time> maxTime) maxTime=testa_valid->time; // credo devo vedere se si può effettuare con qualche istruzione atomica.
                        brelse(bh);
                    }
                    else{
                        __sync_fetch_and_sub(&(testa_valid->countLettore),1);
                        ((myfiledata_struct*)filp->private_data)->num=maxTime;
                        strncpy(string_to_pass+ret,"\0",1);
                        ret=ret+1;
                        printk("1 ret=%d string to pass%s\n",ret ,string_to_pass);
                        int ret1=copy_to_user(buf,string_to_pass,ret);
                        return ret-ret1;
                    }
                }
            }
            __sync_fetch_and_sub(&(testa_valid->countLettore),1);
        }else{ 
            ((myfiledata_struct*)filp->private_data)->num=maxTime;
            strncpy(string_to_pass+ret,"\0",1);
            ret=ret+1;
            printk("2 ret=%d string to pass%s\n",ret ,string_to_pass);
            int ret1=copy_to_user(buf,string_to_pass,ret);
            return ret-ret1;
        }
    }
    if(ret!=0){
        ((myfiledata_struct*)filp->private_data)->num=maxTime;
        strncpy(string_to_pass+ret,"\0",1);
        ret=ret+1;
        int ret1=copy_to_user(buf,string_to_pass,ret);
        printk("3 ret=%d string to pass%s\n",ret ,string_to_pass);
        return ret-ret1;
    }
        ((myfiledata_struct*)filp->private_data)->num=maxTime;
    return ret;
}

int myfileops_open(struct inode * inode, struct file * file){
    myfiledata_struct * fdata=(myfiledata_struct*) kzalloc(sizeof(myfiledata_struct),0);
    if(fdata == NULL) printk("Error\n");
    fdata->num=0;
    int error = stream_open(inode,file);
    file->private_data=(void*) fdata;
    printk("fdata->num: %d\n",fdata->num);
    return error; 
}
int myfileops_release(struct inode * inode, struct file * file){
    kfree(file->private_data);
    printk("free della struct del file privata\n");
    return 0; 
}

/*
int myinodeops_create(struct inode * inode,struct dentry * d,umode_t bool){

}
*/

/*
ssize_t myfileops_read(struct file * filp, char __user * buf, size_t len, loff_t * off) {
        //char * p ="proviamo a far leggere questo";  
        //for(i=0;i<NBLOCK;i++)if(miavariabile[i].infoblock.valid) copia(miavariabile[i].data);
        printk("sto provando a leggere %s e len %d",p,len);
        int index_block_start = *off % (MAXBLOCKDATA);
        if( index_data_block => NBLOCK ) return 0;
        //dopo devo vedere se posso evitare la speculazione...
        int index_block_end = (*off +len) % (MAXBLOCKDATA);
        if( index_block_end => NBLOCK) return 0;
        int byte_start = (*off ) - index_block_start*(MAXBLOCKDATA);
        int byte_end = (*off + len) -index_block_end *(MAXBLOCKDATA);
        int i=0;
        ssize_t ret;
        ssize_t inc=0;
        for(;i<NBLOCK;i++){
            if (data[i].block_information.valid == 1){
                if (ret < *off ){
                    if ( ret + data[i].block_information.dimension <= *off)
                        ret = ret +data[i].block_information.dimension;
                    else ret = *off;

                }else{
                    break;
                }
            }
        }
        while( inc<len || i<NBLOCK ){
            if(data[i].block_information.valid==1){
                if(inc<len){
                    if (inc+data_block_number[i].block_information.dimension <= len){
                        int ret1 = copy_to_user(buf+inc,data_block_number[i].data,data_block_number[i].block_information.dimension );
                        if(ret1 !=0)
                            return inc+(data[i].block_information.dimension - ret1);
                        else
                            inc=inc + data[i].block_information.dimension;
                    }else{
                    
                    }
                }
                else return inc;
            }
            i++;
            
      }

}
*/

struct dentry *onefilefs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags) {

    struct_MyInode *FS_specific_inode;
    struct super_block *sb = parent_inode->i_sb;
    struct buffer_head *bh = NULL;
    struct inode *the_inode = NULL;

    printk("%s: running the lookup inode-function for name %s",MOD_NAME,child_dentry->d_name.name);

    if(!strcmp(child_dentry->d_name.name, UNIQUE_FILE_NAME)){

	
	//get a locked inode from the cache 
        the_inode = iget_locked(sb, 1);
        if (!the_inode)
       		 return ERR_PTR(-ENOMEM);

	//already cached inode - simply return successfully
	if(!(the_inode->i_state & I_NEW)){
		return child_dentry;
	}


	//this work is done if the inode was not already cached
	inode_init_owner(&nop_mnt_idmap,the_inode, NULL, S_IFREG );
	the_inode->i_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IXUSR | S_IXGRP | S_IXOTH;
        the_inode->i_fop = &onefilefs_file_operations;
	the_inode->i_op = &onefilefs_inode_ops;

	//just one link for this file
	set_nlink(the_inode,1);

	//now we retrieve the file size via the FS specific inode, putting it into the generic inode
    	bh = (struct buffer_head *)sb_bread(sb, 1 );
    	if(!bh){
		iput(the_inode);
		return ERR_PTR(-EIO);
    	}
	FS_specific_inode = (struct struct_MyInode*)bh->b_data;
	the_inode->i_size = FS_specific_inode->file_size;
        brelse(bh);

        d_add(child_dentry, the_inode);
	dget(child_dentry);

	//unlock the inode to make it usable 
    	unlock_new_inode(the_inode);

	return child_dentry;
    }

    return NULL;

}

//look up goes in the inode operations
const struct inode_operations onefilefs_inode_ops = {
    .lookup = onefilefs_lookup,
};

const struct file_operations onefilefs_file_operations = {
    .owner = THIS_MODULE,
    .read = myfileops_read,
    //.read = onefilefs_read,
    .open = myfileops_open,
    .release = myfileops_release,
    //.write = onefilefs_write //please implement this function to complete the exercise
};
