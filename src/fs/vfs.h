#pragma once

/* Copyright © 2010, 2011 Lukas Martini
 *
 * This file is part of Xelix.
 *
 * Xelix is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Xelix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Xelix.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <lib/generic.h>
#include <arch/i386/lib/multiboot.h>

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08 // Is the file an active mountpoint?

struct vfs_node;

typedef size_t (*read_type_t)(struct vfs_node*, uint32_t, uint32_t, uint8_t*);
typedef uint32_t (*write_type_t)(struct vfs_node*, uint32_t, uint32_t, uint8_t*);
typedef uint32_t (*open_type_t)(struct vfs_node*);
typedef void (*close_type_t)(struct vfs_node*);
typedef struct dirent* (*readdir_type_t)(struct vfs_node*, uint32_t);
typedef struct vfs_node* (*finddir_type_t)(struct vfs_node*, char* name);

// __fsNode is only there for use in the function declarations above.
typedef struct vfs_node
{
   char name[128];     // The filename.
   uint32_t mask;        // The permissions mask.
   uint32_t uid;         // The owning user.
   uint32_t gid;         // The owning group.
   uint32_t flags;       // Includes the node type. See #defines above.
   uint32_t inode;       // This is device-specific - provides a way for a filesystem to identify files.
   uint32_t length;      // Size of the file, in bytes.
   uint32_t impl;        // An implementation-defined number.
   read_type_t read;
   write_type_t write;
   open_type_t open;
   close_type_t close;
   readdir_type_t readdir;
   finddir_type_t finddir;
   struct vfs_node* ptr; // Used by mountpoints and symlinks.
   struct vfs_node* parent;
} vfs_node_t;

struct dirent // One of these is returned by the readdir call, according to POSIX.
{
  char name[128]; // Filename.
  uint32_t ino;     // Inode number. Required by POSIX.
};

vfs_node_t* vfs_create_node(char name[128], uint32_t mask, uint32_t uid, uint32_t gid, uint32_t flags, uint32_t inode, uint32_t length, uint32_t impl, read_type_t read, write_type_t write, open_type_t open, close_type_t close, readdir_type_t readdir, finddir_type_t finddir, vfs_node_t *ptr, vfs_node_t *parent);

vfs_node_t* vfs_rootnode; // Our root directory node.
vfs_node_t** vfs_rootnodes; // Our root directory array.
int vfs_rootnodecount;

void vfs_init();
