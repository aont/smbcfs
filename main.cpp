#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstddef>
#include <cerrno>

#include <sys/time.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include <libsmbclient.h>

#include <pthread.h>

#include <string>

#define SMBCFS_DEBUG

#define MUTEX_LOCK

const static std::string smbcfs_root_path = "smb://localhost/share";
static pthread_mutex_t mutex_lock;

static int smbcfs_getattr(const char* const path, struct stat* const stbuf)
{

  const std::string smbcfs_path = smbcfs_root_path + path;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_stat = smbc_stat(smbcfs_path.c_str(), stbuf);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_stat] %s %p => %d\n", smbcfs_path.c_str(), stbuf, ret_stat);
#endif



  switch(ret_stat) {
  case -1:
    return -ENOENT;
  default:
    return ret_stat;
  }
}

// static int smbcfs_readlink(const char *, char *, size_t);
// static int smbcfs_mknod(const char *, mode_t, dev_t);

static int smbcfs_mkdir(const char* const path, const mode_t mode)
{

  const std::string smbcfs_path = smbcfs_root_path + path;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_mkdir = smbc_mkdir(smbcfs_path.c_str(), mode);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_mkdir] %s %d => %d\n",smbcfs_path.c_str(), mode, ret_mkdir);
#endif



  return ret_mkdir;
}

static int smbcfs_unlink(const char* const path)
{


  const std::string smbcfs_path = smbcfs_root_path + path;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_unlink = smbc_unlink(smbcfs_path.c_str());

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_unlink] %s => %d\n", smbcfs_path.c_str(), ret_unlink);
#endif


  return ret_unlink;

}

static int smbcfs_rmdir(const char* const path)
{

  const std::string smbcfs_path = smbcfs_root_path + path;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_rmdir = smbc_rmdir(smbcfs_path.c_str());

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_rmdir] %s => %d\n", smbcfs_path.c_str(), ret_rmdir);
#endif

  return ret_rmdir;
}

// static int smbcfs_symlink(const char *, const char *);

static int smbcfs_rename(const char* const ourl, const char* const nurl)
{

  const std::string smb_ourl = smbcfs_root_path + ourl;
  const std::string smb_nurl = smbcfs_root_path + nurl;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_rename = smbc_rename(smb_ourl.c_str(), smb_nurl.c_str());

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_rename] %s %s => %d\n",
	  smb_ourl.c_str(), smb_nurl.c_str(), ret_rename);
#endif

  return ret_rename;

}			

// static int smbcfs_link(const char *, const char *);

static int smbcfs_chmod(const char* const path, const mode_t mode)
{

  const std::string smbcfs_path = smbcfs_root_path + path;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_chmod = smbc_chmod(smbcfs_path.c_str(), mode);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_chmod] %s %d => %d\n",
	  smbcfs_path.c_str(), mode, ret_chmod);
#endif

  return ret_chmod;

}

// static int smbcfs_chown(const char *, uid_t, gid_t)

// static int smbcfs_truncate(const char* const path, const off_t size)

static int smbcfs_utime(const char* const path, struct utimbuf* const utbuf)
{

  const std::string smbcfs_path = smbcfs_root_path + path;

  struct timeval tbuf[2];
  tbuf[0].tv_sec = utbuf->actime;
  tbuf[1].tv_sec = utbuf->modtime;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_utimes = smbc_utimes(smbcfs_path.c_str(), tbuf);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_utimes] %s %p => %d\n",
	  smbcfs_path.c_str(), tbuf, ret_utimes);
#endif
  
  return ret_utimes;

}

static int smbcfs_open(const char* const path, struct fuse_file_info* const fi)
{

  const std::string smbcfs_path = smbcfs_root_path + path;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_open = smbc_open(smbcfs_path.c_str(), fi->flags, 0);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_open] %s %p => %d\n",
	  smbcfs_path.c_str(), fi, ret_open);
#endif

  if(ret_open<0) { return ret_open; }

  fi->fh = ret_open;
  return 0;

}

static int smbcfs_read(const char* const path, char* const buf, const std::size_t size, const off_t offset, struct fuse_file_info* const fi)
{

  const int fh = fi->fh;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_lseek = smbc_lseek(fh, offset, SEEK_SET);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_lseek] %d %lld %d => %d\n",
	  fh, offset, SEEK_SET, ret_lseek);
#endif

  if(ret_lseek<0) return ret_lseek;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_read = smbc_read(fh, buf, size);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_read] %d %p %ld => %d\n",
	  fh, buf, size, ret_read);
#endif

  return ret_read;

}

static int smbcfs_write(const char* const path, const char* const buf, const std::size_t size, const off_t offset, struct fuse_file_info* const fi)
{

  const int fh = fi->fh;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_lseek = smbc_lseek(fh, offset, SEEK_SET);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_lseek] %d %lld %d => %d\n",
	  fh, offset, SEEK_SET, ret_lseek);
#endif

  if(ret_lseek<0) { return ret_lseek; }

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_write = smbc_write(fh, (void*)buf, size);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_write] %d %p %ld => %d\n",
	  fh, buf, size, ret_write);
#endif

  return ret_write;
}

// static int smbcfs_statfs(const char *, struct statvfs *);
// static int smbcfs_flush(const char *, struct fuse_file_info *);

static int smbcfs_release(const char* const path, struct fuse_file_info* const fi)
{

  const int fh = fi->fh;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_close = smbc_close(fh);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_close] %d => %d\n",
	  fh, ret_close);
#endif


  return ret_close;
}

// static int smbcfs_fsync(const char *, const int, struct fuse_file_info *);

static int smbcfs_setxattr
#ifndef __APPLE__
(const char* const path, const char* const name, const char* const value, const size_t size, const int flags)
#else
(const char* const path, const char* const name, const char* const value, const size_t size, const int flags, const uint32_t)
#endif
{

  const std::string smbcfs_path = smbcfs_root_path + path;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_setxattr = smbc_setxattr(smbcfs_path.c_str(), name, value, size, flags);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_setxattr] %s %s %s %ld %d => %d\n",
	  smbcfs_path.c_str(), name, value, size, flags, ret_setxattr);  
#endif

  return ret_setxattr;

}	

static int smbcfs_getxattr
#ifndef __APPLE__
(const char* const path, const char* const name, char* const value, const size_t size)
#else
(const char* const path, const char* const name, char* const value, const size_t size , const uint32_t)
#endif
{

  const std::string smbcfs_path = smbcfs_root_path + path;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_getxattr = smbc_getxattr(smbcfs_path.c_str(), name, value, size);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_setxattr] %s %s %s %ld => %d\n",
	  smbcfs_path.c_str(), name, value, size, ret_getxattr);
#endif
  
  return ret_getxattr;

}

static int smbcfs_listxattr
(const char* const path, char* const list, const size_t size)
{

  const std::string smbcfs_path = smbcfs_root_path + path;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_listxattr = smbc_listxattr(smbcfs_path.c_str(), list, size);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_listxattr] %s %s %ld => %d\n",
	  smbcfs_path.c_str(), list, size, ret_listxattr);
#endif

  return ret_listxattr;

}

static int smbcfs_removexattr(const char* const path, const char* const name)
{

  const std::string smbcfs_path = smbcfs_root_path + path;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_removexattr = smbc_removexattr(smbcfs_path.c_str(), name);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_listxattr] %s %s => %d\n",
	  smbcfs_path.c_str(), name, ret_removexattr);
#endif

  return ret_removexattr;
}

static int smbcfs_opendir(const char *path, struct fuse_file_info *fi)
{

  const std::string smbcfs_path = smbcfs_root_path + path;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_open = smbc_opendir(smbcfs_path.c_str());

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_opendir] %s => %d\n",
	  smbcfs_path.c_str(), ret_open);
#endif

  if(ret_open<0) {
    return ret_open;
  }
  fi->fh = ret_open;

  return 0;
}

static int smbcfs_readdir(const char* const path, void* const buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* const fi)
{

  const int fh = fi->fh;

  while(true) {

#ifdef MUTEX_LOCK
    pthread_mutex_lock(&mutex_lock);  
#endif

    const smbc_dirent* const dirent = smbc_readdir(fh);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
    fprintf(stderr, "[smbc_readdir] %d => %p\n"
	    , fh, dirent);
#endif

    if(dirent==NULL)
      break;

    const unsigned int smbc_type = dirent->smbc_type;
    switch(smbc_type) {
    case SMBC_DIR:
    case SMBC_FILE: {

      const int ret_filler = filler(buf, dirent->name, NULL, 0);

#ifdef SMBCFS_DEBUG
      fprintf(stderr, "[fill_dir] %s %s => %d\n", dirent->name, dirent->comment, ret_filler);
#endif

      if(ret_filler!=0) {
	return -ENOMEM;
      }
      break;
    }
    default:
#ifdef SMBCFS_DEBUG
      fprintf(stderr, "[dirent not used] %s%s\n", dirent->name, dirent->comment);
#endif
      break;
    }

  }
  
  return 0;
}

static int smbcfs_releasedir(const char* const path, struct fuse_file_info* const fi)
{
  const int fh = fi->fh;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_closedir = smbc_closedir(fh);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_closedir] %d => %d\n", fh, ret_closedir);
#endif
  return ret_closedir;
}

// static int smbcfs_fsyncdir(const char *, int, struct fuse_file_info *);

static void get_auth_data_fn(const char* const srv, const char* const shr, char* const wg, const int wglen, char* const un, const int unlen, char* const pw, const int pwlen)
{
  sprintf(pw, "%s", "password");
  // strncpy(pw, "", pwlen - 1);
  sprintf(un, "%s", "user");

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[%s]\n", __func__);

  fprintf(stderr, "  Address\tsmb://%s/%s\n", srv, shr);
  fprintf(stderr, "  Workgroup\t%s\n", wg);
  fprintf(stderr, "  User\t%s\n",  un);
  fprintf(stderr, "  Password\t%s\n",  "***");
#endif



}


static void* smbcfs_init(struct fuse_conn_info *conn)
{
  const int debug_level = 0;

  const int ret_init = smbc_init(get_auth_data_fn, debug_level);

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_init] %p %d => %d\n", get_auth_data_fn, debug_level, ret_init);
#endif
  if(ret_init<0) exit(-ret_init);
  
#ifdef MUTEX_LOCK
  pthread_mutex_init(&mutex_lock, NULL);
#endif

  return NULL;
}

static void smbcfs_destroy(void *)
{

#ifdef MUTEX_LOCK
  pthread_mutex_destroy(&mutex_lock);
#endif

}

// static int smbcfs_access(const char *, int);

static int smbcfs_create(const char* const path, const mode_t mode, struct fuse_file_info* const fi)
{

  const std::string smbcfs_path = smbcfs_root_path + path;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_creat = smbc_creat(smbcfs_path.c_str(), mode);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_creat] %s %d => %d\n", smbcfs_path.c_str(), mode, ret_creat);
#endif

  if(ret_creat<0) { return ret_creat; }

  fi->fh = ret_creat;
  return 0;
}

static int smbcfs_ftruncate(const char* const path, const off_t size, struct fuse_file_info* const fi)
{

  const int fh = fi->fh;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_ftruncate = smbc_ftruncate(fh, size);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_ftruncate] %d %lld => %d\n", fh, size, ret_ftruncate);
#endif
  return ret_ftruncate;
}

static int smbcfs_fgetattr(const char* const path, struct stat* const stbuf, struct fuse_file_info* const fi)
{

  const int fh = fi->fh;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_fstat = smbc_fstat(fh, stbuf);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_fstat] %d %p => %d\n", fh, stbuf, ret_fstat);
#endif
  return ret_fstat;
}

//static int smbcfs_lock(const char *, struct fuse_file_info *, int cmd, struct flock *);

static int smbcfs_utimens(const char* const path, const struct timespec tv[2])
{

  const std::string smbcfs_path = smbcfs_root_path + path;

  struct timeval tbuf[2];
  tbuf[0].tv_sec = tv[0].tv_sec;
  tbuf[0].tv_usec = tv[0].tv_nsec/1000;
  tbuf[1].tv_sec = tv[1].tv_sec;
  tbuf[1].tv_usec = tv[1].tv_nsec/1000;

#ifdef MUTEX_LOCK
  pthread_mutex_lock(&mutex_lock);  
#endif

  const int ret_utimes = smbc_utimes(smbcfs_path.c_str(), tbuf);

#ifdef MUTEX_LOCK
  pthread_mutex_unlock(&mutex_lock);  
#endif

#ifdef SMBCFS_DEBUG
  fprintf(stderr, "[smbc_utimes] %s (%ld, %ld) => %d\n",
	  smbcfs_path.c_str(), tbuf[0].tv_sec, tbuf[1].tv_sec, ret_utimes);
#endif
  
  return ret_utimes;
}

// static int smbcfs_bmap(const char *, size_t blocksize, uint64_t *idx);

// static int smbcfs_ioctl(const char *, int cmd, void *arg, struct fuse_file_info *, unsigned int flags, void *data);

// int smbcfs_poll(const char *, struct fuse_file_info *, struct fuse_pollhandle *ph, unsigned *reventsp);

static fuse_operations smbcfs_oprations;

static int smbcfs_main(const int argc, char* argv[])
{

  smbcfs_oprations.getattr = smbcfs_getattr;
  smbcfs_oprations.mkdir = smbcfs_mkdir;
  smbcfs_oprations.unlink = smbcfs_unlink;
  smbcfs_oprations.rmdir = smbcfs_rmdir;
  smbcfs_oprations.rename = smbcfs_rename;
  smbcfs_oprations.chmod = smbcfs_chmod;
  //smbcfs_oprations.utime = smbcfs_utime;
  (void)smbcfs_utime;

  smbcfs_oprations.open = smbcfs_open;
  smbcfs_oprations.write = smbcfs_write;
  smbcfs_oprations.read = smbcfs_read;
  smbcfs_oprations.release = smbcfs_release;

  smbcfs_oprations.setxattr = smbcfs_setxattr;
  smbcfs_oprations.getxattr = smbcfs_getxattr;
  smbcfs_oprations.listxattr = smbcfs_listxattr;
  smbcfs_oprations.removexattr = smbcfs_removexattr;

  smbcfs_oprations.opendir = smbcfs_opendir;
  smbcfs_oprations.readdir = smbcfs_readdir;
  smbcfs_oprations.releasedir = smbcfs_releasedir;

  smbcfs_oprations.init = smbcfs_init;
  smbcfs_oprations.destroy = smbcfs_destroy;

  smbcfs_oprations.create = smbcfs_create;
  smbcfs_oprations.ftruncate = smbcfs_ftruncate;
  smbcfs_oprations.fgetattr = smbcfs_fgetattr;
  smbcfs_oprations.utimens = smbcfs_utimens;


  return fuse_main(argc, argv, &smbcfs_oprations, NULL);
}

int main(const int argc, char* argv[])
{
  return smbcfs_main(argc, argv);
}
