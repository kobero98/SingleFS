#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>
#include "SingleFileSystem.h"

ssize_t myfileops_read(struct file * filp, char __user * buf, size_t len, loff_t * off){  
    int ret,x;
    struct buffer_head *bh;
    block_file_struct * block;
    metadati_block_element * p;
    myfiledata_struct* s;
    atomic_register *info;
    registro_atomico* reg;
    char * strcapo,* string_to_pass;
    uint64_t startTime, readerTime;
    struct super_block * sb;
    if(len<0) return -EINVAL;
    strcapo ="\n";
    ret=0;
    s = (myfiledata_struct*)filp->private_data;
    startTime = s->time;
    readerTime=getTime();
    string_to_pass = kmalloc(len,0);//GFP_KERNEL
    sb = filp->f_inode->i_sb;
    info =(atomic_register*) sb->s_fs_info;
    reg=info->reg;
    __sync_fetch_and_add(&(reg->num_entry),1);
    if(info->testa == NULL)
    { 
        kfree(string_to_pass);
        __sync_fetch_and_add(&(reg->num_exit),1);
        return -ENODATA;
    }
    p=info->testa;
    do{
        if(checkBit(p->block.index_block)){
            if(p->block.time>startTime && p->block.time<=readerTime){
                bh=sb_bread(sb,2+NUMEROMETADATABLOCK+p->block.index_block);
                block =(block_file_struct *) bh->b_data;
                if(block->size+ret<len){
                        memcpy(string_to_pass+ret,block->dati,block->size);
                        ret=ret+block->size;
                        memcpy(string_to_pass+ret,strcapo,1);
                        ret=ret+1;
                        s->time=p->block.time; //aggiorno il tempo della transazione  
                }
                startTime=p->block.time;
                brelse(bh);
            }
        }
        p=p->next;
    }while(p != NULL); //per ora scorriamo tutta la lista
    __sync_fetch_and_add(&(reg->num_exit),1);
    x=0;
    if(ret!=0) 
        x=copy_to_user(buf,string_to_pass,ret);
    kfree(string_to_pass);
    return ret-x;
}
int myfileops_open(struct inode * inode, struct file * file){
    int error;
    myfiledata_struct * fdata=(myfiledata_struct*) kzalloc(sizeof(myfiledata_struct),0);
    if(fdata == NULL){
        return -ENOMEM;
    }
    fdata->time=0;
    error = stream_open(inode,file);
    file->private_data=(void*) fdata;
    return error; 
}
int myfileops_release(struct inode * inode, struct file * file){
    kfree(file->private_data);
    return 0; 
}
struct dentry *onefilefs_lookup(struct inode *parent_inode, struct dentry *child_dentry, unsigned int flags) {
    struct_MyInode *FS_specific_inode;
    struct super_block *sb;
    struct buffer_head *bh;
    struct inode *the_inode;

    sb = parent_inode->i_sb;
    bh=NULL;
    the_inode=NULL;
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
    #if LINUX_VERSION_CODE <= KERNEL_VERSION(5,12,0)
        inode_init_owner(root_inode, NULL, S_IFDIR);
    #elif LINUX_VERSION_CODE < KERNEL_VERSION(6,3,0)
        inode_init_owner(&init_user_ns,root_inode, NULL, S_IFDIR);
    #elif LINUX_VERSION_CODE >= KERNEL_VERSION(6,3,0)
        inode_init_owner(&nop_mnt_idmap,root_inode, NULL, S_IFDIR);
    #endif
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
    .open = myfileops_open,
    .release = myfileops_release,
};
