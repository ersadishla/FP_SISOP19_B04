#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

static const char *dirpath = "/home/aeris/Music";

typedef struct Node 
{ 
    char key[100];
    char lookfor[100];
    char mpath[1000]; 
    struct Node *left, *right, *down; 
}node; 
   
node *root = NULL;

// A utility function to create a new BST node 
node *newNode(char item[], const char path[]) 
{ 
    node *temp =  (node*)malloc(sizeof(node));
    char porary[1000];
    char tmp[100];
    strcpy(porary, path);
    strcat(porary, "/");
    strcat(porary, item);
    strcpy(temp->mpath, porary);  

    strcpy(tmp, "/");
    strcat(tmp, item);
    strcpy(temp->lookfor, tmp);

    strcpy(temp->key, item);

    temp->left = temp->right = NULL; 
    return temp; 
} 
   
// A utility function to do inorder traversal of BST 
void inorder(node *root) 
{ 
    if (root != NULL) 
    { 
        inorder(root->left); 
        printf("%s\n", root->lookfor); 
        inorder(root->right); 
    } 
} 

 
/* A utility function to insert a new node with given key in BST */
node* insert(node* node, char key[], const char path[]) 
{ 
    /* If the tree is empty, return a new node */
    if (node == NULL) return newNode(key, path); 
  
    /* Otherwise, recur down the tree */
    if (strcmp(key, node->key) < 0) 
        node->left  = insert(node->left, key, path); 
    else if (strcmp(key, node->key) > 0) 
        node->right = insert(node->right, key, path);    
  
    /* return the (unchanged) node pointer */
    return node; 
}

int exist(node* leaf, char key[])
{
    int res;
    if( leaf != NULL ) {
        res = strcmp(leaf->key, key);
        if( res < 0)
            exist(leaf->left, key);
        else if( res > 0)
            exist(leaf->right, key);
        else
            return 1;
    }
    else 
        return 0;
}


// C function to search a given key in a given BST 
char *search(node* root, const char key[]) 
{ 
    // Base Cases: root is null or key is present at root 
    if (root == NULL)
        return "";

    if(strcmp(root->lookfor, key) == 0) 
        return root->mpath; 
     
    // Key is greater than root's key 
    if (strcmp(root->lookfor, key) < 0) 
       return search(root->right, key); 
  
    // Key is smaller than root's key 
    return search(root->left, key); 
} 

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	char fpath[1000];
    char *temp;

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else {
        temp = search(root, path);
        sprintf(fpath, "%s", temp);
    }
    // printf("FPATHHHHHHHHHHHHHHHHHHHHHHHHH : %s\n", fpath);
    int res;
    res = lstat(fpath, stbuf);
    // printf("RESSSSSSSSSSSSSSSSSSSSSSSSSS : %d\n", res);/
    if (res == -1)
        return -errno;

    
    // inorder(root);

	return 0;
}

static int xmp_readagain(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    if (strcmp(path, "/home/aeris/fp/mounted") == 0) return 0;

    int res = 0;
	DIR *dp;
	struct dirent *dir;

	(void) offset;
	(void) fi;

	dp = opendir(path);
	if (dp == NULL)
		return -errno;

	while ((dir = readdir(dp)) != NULL) {
        if(dir->d_type != DT_DIR){ // if the type is not directory just print it with blue
            int length = strlen(dir->d_name);
            if (strncmp(dir->d_name + length - 4, ".mp3", 4) == 0){
                // if(!exist(root, dir->d_name)){
                    // printf("INSERTTTTTTTTTTTTT : %s\n", dir->d_name);
                    root = insert(root, dir->d_name, path);
                    struct stat st;
                    memset(&st, 0, sizeof(st));
                    st.st_ino = dir->d_ino;
                    st.st_mode = dir->d_type << 12;
                    res = (filler(buf, dir->d_name, &st, 0));
                        if(res!=0) break;
                // }
            }
        }
        else if(dir->d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 && dir->d_name[0] != '.'){ // if it is a directory
            char d_path[1000]; // here I am using sprintf which is safer than strcat
            sprintf(d_path, "%s/%s", path, dir->d_name);
            xmp_readagain(d_path, buf, filler, offset, fi); // recall with the new path
        }
	}

	closedir(dp);

    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
	if(strcmp(path, "/") == 0)
	{
		path = dirpath;
		sprintf(fpath, "%s", path);
	}
	else sprintf(fpath, "%s%s", dirpath, path);
	int res = 0;
    
    res = xmp_readagain(fpath, buf, filler, offset, fi);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
    char fpath[1000];
    char *temp;
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else {
        temp = search(root, path);
        sprintf(fpath, "%s", temp);
    }
    // printf("FPATHHHHHHHHHHHHHHHHHHHHHHHHH : %s\n", fpath);
	int res = 0;
  	int fd = 0 ;

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
    return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	int res;
    char fpath[1000];
    char *temp;
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else {
        temp = search(root, path);
        sprintf(fpath, "%s", temp);
    }
	res = statvfs(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
    .statfs	    = xmp_statfs,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}