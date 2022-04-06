# `zipfs` - zip 'heap' filesystem
## a c++ libzip wrapper

`zipfs` purpose is to provide a zip archive that serves as a local filesystem that supports custom encryption.

When encryption and/or decryption are set, zipfs will transparently-

- encrypt and add files to the archive from memory or from the OS filesystem.
- decrypt and serve file data to memory or to the OS filesystem.

Encryption and/or decryption are optional and user-defined.

## `zipfs` goals

- provide an efficient and convenient public interface
- address every libzip' point of failure
- have an easy to read and debug implementation
- be portable (windows / linux / mac)

## public interface

##### note: *read-only* and *write* are referring to the archive.

##### note: every public member (as of now) returns a `zipfs_error_t` object that can be tested out for errors.

#### § *write* filesystem operations

- `zipfs_error_t file_pull(...);`

    Pulls a file from the filesystem into the archive.

- `zipfs_error_t file_pull_replace(...);`

    Pulls a file from the filesystem into the archive and replaces the existing one.

- `zipfs_error_t dir_pull(...);`

    Pulls all the contents of a directory into the archive.

- `zipfs_error_t dir_pull_query(...);`

    Runs a query with set `OVERWRITE` and `ORPHAN` flags without modifying anything. Query results can be inspected.

#### § *read-only* filesystem operations

- `zipfs_error_t file_extract(...);`

    Extracts a file from the archive to the filesystem.

- `zipfs_error_t file_extract_replace(...);`

    Extracts a file from the archive to the filesystem and replaces the existing one.

- `zipfs_error_t dir_extract(...);`

    Extracts all the contents of a directory to the filesystem.

- `zipfs_error_t dir_extract_query(...);`

    Runs a query with set `OVERWRITE` flag without modifying anything. Query results can be inspected.

#### § *write* memory operations

- `zipfs_error_t file_add(...);`

    Adds a file to the archive from binary data.

- `zipfs_error_t file_add_replace(...);`

    Adds a file to the archive from binary data and replaces the existing one.

- `zipfs_error_t file_delete(...);`

    Deletes a file from the archive.

- `zipfs_error_t file_rename(...);`

    Renames a file in the archive.

- `zipfs_error_t dir_add(...);`

    Adds a directory to the archive. The whole directory tree will be created if necessary.

- `zipfs_error_t dir_delete(...);`

    Deletes a directory from the archive. The whole contents will be deleted.

#### § *read-only* memory operations

- `zipfs_error_t cat(...);`

    Retrieves binary data for a file.

- `zipfs_error_t ls(...);`

    Lists the contents of a dir.

- `zipfs_error_t stat(...);`

    Retrieves the zip information of an entry.

- `zipfs_error_t index(...);`

    Retrieves the zip index of an entry.

- `zipfs_error_t num_entries(...);`

    Retrieves the total entry count of the archive.

#### § source data

- `zipfs_error_t get_source(...);`

    Retrieves the archive source data. Can be written directly on disk.

- `zipfs_error_t zip_source_t_has_modifications(...);`

    Compares source data to source data image.

- `zipfs_error_t zip_source_t_revert_to_image(void);`

    Sets source data to source data image.

- `zipfs_error_t zip_source_t_image_update(void);`

    Sets source data image to source data.

#### § compression

- `void set_compression(...);`

    Sets the compression method to use from now on - doesn't modify the archive. libzip will remember which compression method was used for what file. You can build libzip with support for various optional compression methods and use them here.

#### § encryption & decryption

- `void set_file_encrypt(bool encrypt);`

    Sets file encryption to active or inactive.

- `void set_file_decrypt(bool decrypt);`

    Sets file decryption to active or inactive.

- `void set_file_encrypt_func(file_encrypt_func f);`

    Sets the file encryption function.

- `void set_file_decrypt_func(file_decrypt_func f);`

    Sets the file decryption function.

- `file_encrypt_func` / `file_decrypt_func`

    The typedef for the file encryption and decryption functions is:

    `typedef void(*f)(const char* filename, const uint8_t* buf, size_t len, uint8_t** ret_buf, size_t* ret_len);`

## tutorials

- tutorial #0

    A short introduction on what you need to know about `zipfs_path_t` and `filesystem_path_t`. All archive paths are handled with `zipfs_path_t`. All OS-filesystem paths are handled with `filesystem_path_t`.

- tutorial #1

    *write* memory operations - this will let us create an archive from scratch and continue from there.

- tutorial #2

    *read-only* memory operations - we will use the archive created in tutorial 1 here.

- tutorial #3

    *read-only* filesystem operations (extracting data).

- tutorial #4

    *write* filesystem operations (pulling data).

- tutorial #5

    encryption & decryption.

- tutorial #6

    a filesystem mirroring script.

## requirements

- C++ 17

## dependencies

- libzip & zlib
- boost (only the `boost::filesystem::operations` header)
- a few functions gathered in the `util::` namespace

## build instructions

- Download and build [`zlib`](https://www.zlib.net/)
- Download and build [`libzip`](https://libzip.org/download/)
- Download and build [`util`](https://github.com/lapinott/util) (& boost)
- Download `zipfs` here
- Generate the build solution with CMAKE. You will have to tell CMAKE where to look for the aformentioned dependencies. 

## disclaimer
`zipfs` aims at providing *functionnality*. It was not written with any particular coding standard in mind. Hopefully the implementation is still easy to read and understand.

## license
(c) 2022 Julien Matthey - Released under the MIT license. Feel free to contribute 😊