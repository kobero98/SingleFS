#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/mnt_idmapping.h>
#include <linux/types.h>

#include "SingleFileSystem.h"

/*
    #ifdef MY_DEBUG
    #define AUDIT if(1)
    #else
    #define AUDIT if(0)
    #endif
*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matteo Federico");
MODULE_DESCRIPTION("un semplice modulo iniziale");
MODULE_VERSION("0.1");


static int countFSmount = 0;
//driver da implementare!
static struct super_operations myFileSystem_super_ops = {};
static struct dentry_operations myFileSystem_dentry_ops={};
static struct inode_operations myFileSystem_inode_ops = {};
static struct file_operations myFileSystem_file_ops = {};
static struct file_operations myFileSystem_dir_ops = {};
void init_struct_block(void);

void init_struct_block(void){
    int i=0;
    for(;i<NBLOCK;i++){
        data[i].block_information.valid=0;
    }
    return ;
}
//funzione per il riempimento del superblocco
int myFileSystem_fill_sb(struct super_block *sb,void*data,int silent){
    struct inode *root_inode;
    struct buffer_head *bh;  
    struct_sb_information *mysb;
    struct timespec64 curr_time;
    uint64_t magic;
    uint64_t nblock;
    sb->s_magic= MAGICNUMBER;
    bh = sb_bread(sb,0); //sostituire 0 con una macro lo 0 indica dove sta posizionato il super block 
    //qui forse ci sta un errore
    if(!sb) return -EIO;
    mysb = (struct_sb_information *)bh->b_data;
    
    //qui il prof ha messo un controllo sul meagic number che non ho ben compreso...
    magic = mysb->magic;
    nblock = mysb->nblock;
    brelse(bh);
    
    if(magic != sb->s_magic) return -EBADF;
    if(nblock != NBLOCK) return -EBADF;

    sb->s_fs_info = NULL;
    sb->s_op = &myFileSystem_super_ops;

    //ora creo l'inode
    root_inode = iget_locked(sb,0);
    if(!root_inode) return  -ENOMEM;

    root_inode->i_ino = 10; //da sostituire con una macro 
    // questa istruzione é dipendendete dalla versione 6.3 la funzione inode_init_owner prende 4 parametri
    printk("qui mi rompo\n");
    inode_init_owner(&nop_mnt_idmap,root_inode, NULL, S_IFDIR);
    printk("qui mi rompo2\n");
    root_inode->i_sb = sb;
    
   // root_inode->i_op = &myFileSystem_inode_ops;//&onefilefs_inode_ops;     
   //root_inode->i_fop = &myFileSystem_dir_ops; //&onefilefs_dir_operations; credo che qui il prof possa essersi sbagliato
    root_inode->i_op = &onefilefs_inode_ops;
    root_inode->i_fop = &onefilefs_dir_operations;

    root_inode ->i_mode = S_IFDIR | S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IXUSR | S_IXGRP |  S_IXOTH;
    //DEVO LEGGERE ASSOLUTMANTE I_mode cosa gli sto dando...

    ktime_get_real_ts64(&curr_time);
    root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = curr_time;

    root_inode->i_private = NULL;
    sb->s_root = d_make_root(root_inode);
    if (!sb->s_root) return -ENOMEM;
    sb->s_root->d_op = &myFileSystem_dentry_ops;
    unlock_new_inode(root_inode);

    init_struct_block();
    printk("mount avvenuta con successo\n");
    return 0;
}

//funzione per il montaggio del filesystem
struct dentry * myFileSystem_mount(struct file_system_type * type, int flags, const char *dev_name,void *data){
    if(! __sync_bool_compare_and_swap(&countFSmount,0,1)){
        printk("non é possibile montare il file system\n");
        return -EIO;
    }
    struct dentry * ret =  mount_bdev(type,flags,dev_name,data,myFileSystem_fill_sb); //forse bisogna utilizzare mount_nodev poi studio meglio.
    if (unlikely(IS_ERR(ret)))
        printk("Errore durante il montaggio del filesystem");
    else
        printk("montaggio avvenuto con successo");
    return ret;
}

//funzione per lo smontaggio del filesystem
static void myFileSystem_kill_sb(struct super_block *sb){
    if(! __sync_bool_compare_and_swap(&countFSmount,1,0)){
        
        printk("non é possibile smontare il file system\n");
        return ;
    }
    kill_block_super(sb);
    printk("file system smontato\n");
    return ;
}

//my file system type kernel 6.3 credo vada bene devo controllare nelle versioni più aggiornate
static struct file_system_type myFileSystemType = {
    .owner=THIS_MODULE,
    .name="MyFileSystem",
    .mount=myFileSystem_mount,//funzione di montaggio del file system
    .kill_sb=myFileSystem_kill_sb,//funzione di smontaggio del file system
    //.fs_flags= FS_NO_DCACHE, //controllare quali altri flag possono essere utili
};

int init_func(void){
    int  ret=register_filesystem(&myFileSystemType);
    printk("il valore della macro é %d\n",NBLOCK);
    if (likely(ret==0))
        printk("file system inserito con successo\n");
    else   
        printk("errore nel montaggio del file system %d\n",ret);
    return ret;
}

void cleanup_func(void){
    int  ret=unregister_filesystem(&myFileSystemType);
    if (likely(ret==0))
        printk("file system smontato con successo\n");
    else    
        printk("errore nel montaggio del file system %d\n",ret);
    return;
}

module_init(init_func);
module_exit(cleanup_func);