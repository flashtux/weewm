# Copyright (c) 2004 FlashCode <flashcode@flashtux.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

OUTPUT=weewm

all:
	cd src && make

install:
	@mkdir -v -p $(DESTDIR)/usr/$(LOCALRPM)/bin
	@cp -v src/$(OUTPUT) $(DESTDIR)/usr/$(LOCALRPM)/bin/
	@mkdir -v -p $(DESTDIR)/usr/share/man/man1
	@cp -v weewm.1 $(DESTDIR)/usr/share/man/man1/
	@echo -e "\n=== WeeWM installed !\n"

clean:
	cd src && make clean
