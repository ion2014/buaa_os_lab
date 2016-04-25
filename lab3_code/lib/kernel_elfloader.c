/* This is a simplefied ELF loader for kernel.
 * You can contact me if you find any bugs.
 *
 * Luming Wang<wlm199558@126.com>
 */

#include <kerelf.h>
#include <types.h>
#include <pmap.h>

/* Overview:
 *   Check whether it is a ELF file.
 *
 * Pre-Condition:
 *   binary must longer than 4 byte.
 *
 * Post-Condition:
 *   Return 0 if `binary` isn't an elf. Otherwise
 * return 1.
 */
int is_elf_format(u_char *binary)
{
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)binary;

	if (ehdr->e_ident[0] == EI_MAG0 &&
		ehdr->e_ident[1] == EI_MAG1 &&
		ehdr->e_ident[2] == EI_MAG2 &&
		ehdr->e_ident[3] == EI_MAG3) {
		return 0;
	}

	return 1;
}

/* Overview:
 *   load an elf format binary file. Map all section
 * at correct virtual address.
 *
 * Pre-Condition:
 *   `binary` can't be NULL and `size` is the size of binary.
 *
 * Post-Condition:
 *   Return 0 if success. Otherwise return < 0.
 *   If success, the entry point of `binary` will be stored in `start`
 */
int load_elf(u_char *binary, int size, u_long *entry_point, void *user_data,
			 int (*map)(u_long va, u_int32_t sgsize,
						u_char *bin, u_int32_t bin_size, void *user_data))
{
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)binary;
	Elf32_Phdr *phdr = NULL;
	/* As a loader, we just care about segment,
	 * so we just parse program headers.
	 */
	u_char *ptr_ph_table = NULL;
	Elf32_Half ph_entry_count;
	Elf32_Half ph_entry_size;
	int r;

	// check whether `binary` is a ELF file.
	if (size < 4 || !is_elf_format(binary)) {
		return -1;
	}

	ptr_ph_table = binary + ehdr->e_phoff;	//将指针移到一个段落开始的地方.
	ph_entry_count = ehdr->e_phnum;			//这个是elf预先生成好的一个数, 所以读取就行了
	ph_entry_size = ehdr->e_phentsize;		//这个是第一个区间的大小  

	while (ph_entry_count--) {
		phdr = (Elf32_Phdr *)ptr_ph_table;	//获取第一段

		if (phdr->p_type == PT_LOAD) {		//如果是加载段
			r = map(phdr->p_vaddr, phdr->p_memsz,
					binary + phdr->p_offset, phdr->p_filesz, user_data);
											//将其映射到物理内存中
			if (r < 0) {
				return r;
			}
		}

		ptr_ph_table += ph_entry_size;		//将指针向后移动一段大小的距离
	}

	*entry_point = ehdr->e_entry;
	return 0;
}
