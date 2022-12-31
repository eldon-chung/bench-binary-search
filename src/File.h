#pragma once

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string_view>

struct File {
  int m_fd;
  std::string_view m_view;

  File(const char *path) {
    m_fd = open(path, O_RDONLY);
    if (m_fd == -1) {
      fprintf(stderr, "error closing file. %s\n", strerror(errno));
      exit(1);
    }

    // stat the file
    struct stat statbuf;
    if (fstat(m_fd, &statbuf) == -1) {
      fprintf(stderr, "error statting file. %s\n", strerror(errno));
      exit(1);
    }

    size_t file_size = (size_t)statbuf.st_size;
    char *addr = (char *)mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, m_fd, 0);
    if ((void *)addr == (void *)-1) {
      fprintf(stderr, "error mapping file. %s\n", strerror(errno));
      exit(1);
    }

    m_view = std::string_view{addr, file_size};
  }

  ~File() {
    if (close(m_fd) == -1) {
      fprintf(stderr, "error closing file. %s\n", strerror(errno));
    }
    if (munmap((void *)m_view.data(), m_view.size()) == -1) {
      fprintf(stderr, "error unmapping file. %s\n", strerror(errno));
    }
  }

  File(File const &) = delete;
  File(File &&) = delete;
  File &operator=(File const &) = delete;
  File &operator=(File &&) = delete;

  uint32_t read32_memcpy(size_t index) const {
    uint32_t to_return = 0;
    memcpy(&to_return, m_view.data() + index * 4, 4);
    return to_return;
  }

  uint64_t read64_memcpy(size_t index) const {
    uint64_t to_return = 0;
    memcpy(&to_return, m_view.data() + index * 8, 8);
    return to_return;
  }

  uint32_t read32_reintc(size_t index) const {
    uint32_t const *data = reinterpret_cast<uint32_t const *>(m_view.data());
    return data[index];
  }

  uint64_t read64_reintc(size_t index) const {
    uint64_t const *data = reinterpret_cast<uint64_t const *>(m_view.data());
    return data[index];
  }

  uint32_t const *addr32(size_t index) const {
    return reinterpret_cast<uint32_t const *>(m_view.data()) + index;
  }
  uint64_t const *addr64(size_t index) const {
    return reinterpret_cast<uint64_t const *>(m_view.data()) + index;
  }

  uint32_t const *begin() const { return addr32(0); }
  uint32_t const *end() const { return addr32(0) + size(32); }

  size_t size(uint8_t width) const { return m_view.size() * 8 / width; }
};
