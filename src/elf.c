#include "elf.h"

#include <elf.h>
#include <string.h>

#define LOAD_ADDRESS_TEXT 0x8048000
//#define LOAD_ADDRESS_DATA 0x8049000

void elf_write(FILE* fd, symbol_t symbols, struct buffer* text, struct buffer* data) {
   const char shstrtab[] = "\0.symtab\0.shstrtab\0.strtab\0.text\0.data\0.rel.text";
   
   // symbol labels
   int sym_count = 0;
   int strtab_size = 1;
   for (symbol_t* s = &symbols; s != NULL; s = s->next) {
      int len = strlen(s->name) + 1;
      strtab_size += len;
      sym_count++;
   }
   int sym_size = sizeof(Elf32_Sym) * sym_count;
   

	/* ELF HEADER */
   Elf32_Ehdr ehdr = {
      /* general */
      .e_ident   = {
        ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3,
        ELFCLASS32, 
        ELFDATA2LSB,
        EV_CURRENT,
        ELFOSABI_LINUX,
      },
      .e_type    = ET_EXEC,
      .e_machine = EM_386,
      .e_version = EV_CURRENT,
      .e_entry   = LOAD_ADDRESS_TEXT + 0x74,
      .e_phoff   = sizeof(Elf32_Ehdr),
      .e_shoff   = sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr) + text->size + data->size + sym_size + strtab_size + sizeof(shstrtab), 
      .e_flags   = 0,
      .e_ehsize   = sizeof (Elf32_Ehdr),
      /* program header */
      .e_phentsize = sizeof(Elf32_Phdr),
      .e_phnum     = 2,
      /* section header */
      .e_shentsize = sizeof (Elf32_Shdr),
      .e_shnum     = 6,
      .e_shstrndx  = 5 
      //.e_shnum     = 0,
      //.e_shstrndx  = 0 
   };
   fwrite (&ehdr, 1, sizeof (Elf32_Ehdr), fd);
   
   // program header
   Elf32_Phdr text_header = {
      .p_type   = PT_LOAD,
      .p_offset = 0,
      .p_vaddr = LOAD_ADDRESS_TEXT,
      .p_paddr = 0,
      .p_filesz = text->size,
      .p_memsz = text->size,
      .p_flags = PF_R | PF_X,
      .p_align = 0x1000
   };
   fwrite(&text_header, 1, sizeof(text_header), fd);
   
   // program header
   Elf32_Phdr data_header = {
      .p_type   = PT_LOAD,
      .p_offset = text->size,
      .p_vaddr = LOAD_ADDRESS_TEXT + text->size,
      .p_paddr = 0,
      .p_filesz = data->size,
      .p_memsz = data->size,
      .p_flags = PF_R | PF_W,
      .p_align = 0x1000
   };
   fwrite(&data_header, 1, sizeof(data_header), fd);
   
   // text
   fwrite(text->data, 1, text->size, fd);
   
   // data
   fwrite(data->data, 1, data->size, fd);
   
     // symbols
   Elf32_Sym sym;

   int sym_name_offset = 1;
   for (symbol_t* s = &symbols; s != NULL; s = s->next) {
      sym.st_name = sym_name_offset;
      sym.st_value = s->text;
      sym.st_size = s->text_size;
      sym.st_info = ELF32_ST_INFO(STB_GLOBAL, STT_FUNC);
      sym.st_other = ELF32_ST_VISIBILITY(STV_DEFAULT);
      sym.st_shndx = 4;
      fwrite (&sym, sizeof(sym), 1, fd); 
      sym_name_offset += strlen(s->name) + 1;
   }
   
   // symbol names
   const char TERM = '\0';
   fwrite (&TERM, 1, 1, fd);
   for (symbol_t* s = &symbols; s != NULL; s = s->next) {
      int len = strlen(s->name) + 1;
      fwrite (s->name, len, 1, fd);
   }
   
   // section name
   fwrite(shstrtab, 1, sizeof(shstrtab), fd);
   
   // null section header
   {
      Elf32_Shdr null_sec;
      memset(&null_sec, 0, sizeof(null_sec));
      fwrite(&null_sec, 1, sizeof(null_sec), fd);
   }
   
   // text section header
   {
      Elf32_Shdr text_sec;
      memset(&text_sec, 0, sizeof(text_sec));
      text_sec.sh_name = 27;
      text_sec.sh_type = SHT_PROGBITS;
      text_sec.sh_flags = SHF_ALLOC | SHF_EXECINSTR;
      text_sec.sh_addr = LOAD_ADDRESS_TEXT + sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr);
      text_sec.sh_offset = sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr);
      text_sec.sh_size = text->size;
      text_sec.sh_link = 0;
      fwrite(&text_sec, 1, sizeof(text_sec), fd);
   }
   
   // data section header
   {
      Elf32_Shdr data_sec;
      memset(&data_sec, 0, sizeof(data_sec));
      data_sec.sh_name = 33;
      data_sec.sh_type = SHT_PROGBITS;
      data_sec.sh_flags = SHF_ALLOC | SHF_WRITE;
      data_sec.sh_addr = LOAD_ADDRESS_TEXT + sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr) + text->size;
      data_sec.sh_offset = sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr) + text->size;
      data_sec.sh_size = data->size;
      data_sec.sh_link = 0;
      fwrite(&data_sec, 1, sizeof(data_sec), fd);
   }
   
   // symbol section header
   {
      Elf32_Shdr sym_sec;
      memset(&sym_sec, 0, sizeof(sym_sec));
      sym_sec.sh_name = 1;
      sym_sec.sh_type = SHT_SYMTAB;
      sym_sec.sh_offset = sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr) + text->size + data->size;
      sym_sec.sh_size = sym_size;
      sym_sec.sh_link = 4;
      sym_sec.sh_entsize = sizeof(Elf32_Sym);
      fwrite(&sym_sec, 1, sizeof(sym_sec), fd);
   }
    
   // symbol string header
   {
      Elf32_Shdr symstr_sec;
      memset(&symstr_sec, 0, sizeof(symstr_sec));
      symstr_sec.sh_name = 19;
      symstr_sec.sh_type = SHT_STRTAB;
      symstr_sec.sh_offset = sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr) + text->size + data->size + sym_size;
      symstr_sec.sh_size = strtab_size;
      fwrite(&symstr_sec, 1, sizeof(symstr_sec), fd);
   }
   
   // section string header
   {
      Elf32_Shdr str_sec;
      memset(&str_sec, 0, sizeof(str_sec));
      str_sec.sh_name = 9;
      str_sec.sh_type = SHT_STRTAB;
      str_sec.sh_offset = sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr) + text->size + data->size + sym_size + strtab_size;
      str_sec.sh_size = sizeof(shstrtab);
      fwrite(&str_sec, 1, sizeof(str_sec), fd);
   }

}
