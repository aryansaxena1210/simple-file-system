CC = gcc
LDLIBS = -lpthread
OBJDIR = build

objects = $(OBJDIR)/api.o $(OBJDIR)/application.o $(OBJDIR)/data_block.o $(OBJDIR)/dir.o $(OBJDIR)/inode.o $(OBJDIR)/open_file_table.o
App = $(OBJDIR)/app

all: $(OBJDIR) $(App)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(App): $(objects)
	$(CC) -o $(App) $(objects) $(LDLIBS)

$(OBJDIR)/%.o: %.c
	$(CC) -c $< -o $@

clean:
	rm -rf $(OBJDIR)