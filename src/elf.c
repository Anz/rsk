#include "elf.h"

#include <elf.h>
#include <string.h>

void elf_write(FILE* fd, symbol_t symbols, unsigned char* code, int code_size, unsigned char* data, int data_size) {
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
   
   int offset_shstrtab = sizeof(Elf32_Ehdr);
   int offset_strtab = offset_shstrtab + sizeof(shstrtab);
   int offset_symbols =  offset_strtab + strtab_size;
   int offset_code = offset_symbols + sym_size;
   int offset_data = offset_code + code_size;
   int offset_sections = offset_data + data_size;

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
      .e_type    = ET_REL,
      .e_machine = EM_386,
      .e_version = EV_CURRENT,
      .e_entry   = 0,
      .e_phoff   = 0,
      .e_shoff   = offset_sections, 
      .e_flags   = 0,
      .e_ehsize   = sizeof (Elf32_Ehdr),
      /* program header */
      .e_phentsize = 0,
      .e_phnum     = 0,
      /* section header */
      .e_shentsize = sizeof (Elf32_Shdr),
      .e_shnum     = 6,
      .e_shstrndx  = 2 
   };
   fwrite (&ehdr, sizeof (Elf32_Ehdr), 1, fd);
   
   // section labels
   fwrite(shstrtab, 1, sizeof(shstrtab), fd);
   
   // symbol labels
   const char TERM = '\0';
   fwrite (&TERM, 1, 1, fd);
   for (symbol_t* s = &symbols; s != NULL; s = s->next) {
      int len = strlen(s->name) + 1;
      fwrite (s->name, len, 1, fd);
   }
   
   // symbols
   Elf32_Sym sym;
   memset(&sym, 0, sizeof(sym));
   
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
   
   // text
   fwrite(code, 1, code_size, fd);
   
   // data
   fwrite(data, 1, data_size, fd);
   
   // section header table
   Elf32_Shdr sections[6];
   memset(sections, 0, sizeof(sections));
   
   // symbol section header
   sections[1].sh_name = 1;
   sections[1].sh_type = SHT_SYMTAB;
   sections[1].sh_offset = offset_symbols;
   sections[1].sh_size = sym_size;
   sections[1].sh_link = 3;
   sections[1].sh_entsize = sizeof(Elf32_Sym);
   
   // section string header
   sections[2].sh_name = 9;
   sections[2].sh_type = SHT_STRTAB;
   sections[2].sh_offset = offset_shstrtab;
   sections[2].sh_size = sizeof(shstrtab);
   
   // symbol string header
   sections[3].sh_name = 19;
   sections[3].sh_type = SHT_STRTAB;
   sections[3].sh_offset = offset_strtab;
   sections[3].sh_size = strtab_size;
   
   // text section header
   sections[4].sh_name = 27;
   sections[4].sh_type = SHT_PROGBITS;
   sections[4].sh_flags = SHF_ALLOC | SHF_EXECINSTR;
   sections[4].sh_offset = offset_code;
   sections[4].sh_size = code_size;
   sections[4].sh_link = 3;
   
   // data section header
   sections[5].sh_name = 33;
   sections[5].sh_type = SHT_PROGBITS;
   sections[5].sh_flags = SHF_ALLOC | SHF_WRITE;
   sections[5].sh_offset = 0;
   sections[5].sh_size = 0;
   sections[5].sh_link = 0;
   
   // relocation section header
/*   sections[5].sh_name = 33;
   sections[5].sh_type = SHT_REL;
   sections[5].sh_offset = 0;
   sections[5].sh_size = 0;
   sections[5].sh_link = 0;
   sections[5].sh_entsize = sizeof(Elf32_Rel);*/
   
   fwrite (sections, sizeof(sections), 1, fd);
}
