PROJ_NAME = ted

CC      = gcc
CFLAGS  = -ggdb -O0 -std=c99 -fstack-protector-all \
           -Werror=implicit-function-declaration \
           -Wall -Wextra -Wpedantic \
           -I../include
LDFLAGS = -O0

ifeq ($(OS), Windows_NT)
    RM = del /Q /F
    EXE = .exe
    PATHSEP = \\
    RUN =
else
    RM = rm -f
    EXE =
    PATHSEP = /
    RUN = ./
endif

# ---------------------------------------------------------------------------
# Objetos do programa principal
# ---------------------------------------------------------------------------
OBJETOS = main.o \
          hash.o \
          quadra.o \
          grafo.o  \
          dijkstra.o \
          componentesConexos.o \
          mst.o \
          svg.o \
          parserGeo.o \
          parserVia.o \
          parserQry.o

# ---------------------------------------------------------------------------
# Unity 
# ---------------------------------------------------------------------------
UNITY_DIR = ../Unity
UNITY_OBJ = $(UNITY_DIR)/unity.o

$(UNITY_OBJ): $(UNITY_DIR)/unity.c
	$(CC) -c $(CFLAGS) -I$(UNITY_DIR) -DUNITY_INCLUDE_DOUBLE $< -o $@

# ---------------------------------------------------------------------------
# Executável principal
# ---------------------------------------------------------------------------
ted: $(OBJETOS)
	$(CC) $(LDFLAGS) -o $(PROJ_NAME)$(EXE) $(OBJETOS)

# ---------------------------------------------------------------------------
# Regras de compilação dos módulos
# ---------------------------------------------------------------------------
main.o: main.c ../include/hash.h ../include/quadra.h ../include/grafo.h \
        ../include/svg.h ../include/parserGeo.h ../include/parserVia.h ../include/parserQry.h
	$(CC) -c $(CFLAGS) $< -o $@

hash.o: hash.c ../include/hash.h
	$(CC) -c $(CFLAGS) $< -o $@

quadra.o: quadra.c ../include/quadra.h
	$(CC) -c $(CFLAGS) $< -o $@

grafo.o: grafo.c ../include/grafo.h ../include/hash.h
	$(CC) -c $(CFLAGS) $< -o $@

dijkstra.o: dijkstra.c ../include/dijkstra.h ../include/grafo.h
	$(CC) -c $(CFLAGS) $< -o $@

componentesConexos.o: componentesConexos.c ../include/componentesConexos.h ../include/grafo.h
	$(CC) -c $(CFLAGS) $< -o $@

mst.o: mst.c ../include/mst.h ../include/grafo.h
	$(CC) -c $(CFLAGS) $< -o $@

svg.o: svg.c ../include/svg.h
	$(CC) -c $(CFLAGS) $< -o $@

parserGeo.o: parserGeo.c ../include/parserGeo.h \
             ../include/hash.h ../include/quadra.h ../include/svg.h
	$(CC) -c $(CFLAGS) $< -o $@

parserVia.o: parserVia.c ../include/parserVia.h ../include/grafo.h
	$(CC) -c $(CFLAGS) $< -o $@

parserQry.o: parserQry.c ../include/parserQry.h \
             ../include/hash.h ../include/quadra.h ../include/grafo.h \
             ../include/dijkstra.h ../include/svg.h \
             ../include/componentesConexos.h ../include/mst.h
	$(CC) -c $(CFLAGS) $< -o $@

# ---------------------------------------------------------------------------
# Flags de compilação dos testes
# ---------------------------------------------------------------------------
TST_FLAGS = $(CFLAGS) -I$(UNITY_DIR) -DUNITY_INCLUDE_DOUBLE

# ---------------------------------------------------------------------------
# Testes unitários
# ---------------------------------------------------------------------------

../test/test_hash$(EXE): ../test/test_hash.c hash.o $(UNITY_OBJ)
	$(CC) $(TST_FLAGS) $^ -o $@

test_hash: ../test/test_hash$(EXE)
	$(RUN)../test$(PATHSEP)test_hash$(EXE)

../test/test_quadra$(EXE): ../test/test_quadra.c quadra.o $(UNITY_OBJ)
	$(CC) $(TST_FLAGS) $^ -o $@

test_quadra: ../test/test_quadra$(EXE)
	$(RUN)../test$(PATHSEP)test_quadra$(EXE)

../test/test_grafo$(EXE): ../test/test_grafo.c grafo.o hash.o $(UNITY_OBJ)
	$(CC) $(TST_FLAGS) $^ -o $@

test_grafo: ../test/test_grafo$(EXE)
	$(RUN)../test$(PATHSEP)test_grafo$(EXE)

../test/test_dijkstra$(EXE): ../test/test_dijkstra.c dijkstra.o grafo.o hash.o $(UNITY_OBJ)
	$(CC) $(TST_FLAGS) $^ -o $@

test_dijkstra: ../test/test_dijkstra$(EXE)
	$(RUN)../test$(PATHSEP)test_dijkstra$(EXE)

../test/test_componentesConexos$(EXE): ../test/test_componentesConexos.c componentesConexos.o \
                                        grafo.o hash.o $(UNITY_OBJ)
	$(CC) $(TST_FLAGS) $^ -o $@

test_componentesConexos: ../test/test_componentesConexos$(EXE)
	$(RUN)../test$(PATHSEP)test_componentesConexos$(EXE)

../test/test_mst$(EXE): ../test/test_mst.c mst.o grafo.o hash.o $(UNITY_OBJ)
	$(CC) $(TST_FLAGS) $^ -o $@

test_mst: ../test/test_mst$(EXE)
	$(RUN)../test$(PATHSEP)test_mst$(EXE)

../test/test_svg$(EXE): ../test/test_svg.c svg.o quadra.o hash.o $(UNITY_OBJ)
	$(CC) $(TST_FLAGS) $^ -o $@

test_svg: ../test/test_svg$(EXE)
	$(RUN)../test$(PATHSEP)test_svg$(EXE)

../test/test_parserGeo$(EXE): ../test/test_parserGeo.c parserGeo.o \
                               hash.o quadra.o svg.o $(UNITY_OBJ)
	$(CC) $(TST_FLAGS) $^ -o $@

test_parserGeo: ../test/test_parserGeo$(EXE)
	$(RUN)../test$(PATHSEP)test_parserGeo$(EXE)

../test/test_parserVia$(EXE): ../test/test_parserVia.c parserVia.o grafo.o hash.o $(UNITY_OBJ)
	$(CC) $(TST_FLAGS) $^ -o $@

test_parserVia: ../test/test_parserVia$(EXE)
	$(RUN)../test$(PATHSEP)test_parserVia$(EXE)

../test/test_parserQry$(EXE): ../test/test_parserQry.c parserQry.o \
                               hash.o quadra.o grafo.o dijkstra.o \
                               componentesConexos.o mst.o svg.o $(UNITY_OBJ)
	$(CC) $(TST_FLAGS) $^ -o $@

test_parserQry: ../test/test_parserQry$(EXE)
	$(RUN)../test$(PATHSEP)test_parserQry$(EXE)

# ---------------------------------------------------------------------------
# Roda todos os testes
# ---------------------------------------------------------------------------
tstall: test_hash test_quadra test_grafo test_dijkstra \
        test_componentesConexos test_mst test_svg \
        test_parserGeo test_parserVia test_parserQry

# ---------------------------------------------------------------------------
# Limpeza 
# ---------------------------------------------------------------------------
ifeq ($(OS), Windows_NT)
clean:
	-del /Q /F *.o $(PROJ_NAME).exe ..\Unity\unity.o 2>nul
	-del /Q /F ..\test\test_hash.exe ..\test\test_quadra.exe ..\test\test_grafo.exe 2>nul
	-del /Q /F ..\test\test_dijkstra.exe ..\test\test_componentesConexos.exe ..\test\test_mst.exe 2>nul
	-del /Q /F ..\test\test_svg.exe ..\test\test_parserGeo.exe ..\test\test_parserVia.exe ..\test\test_parserQry.exe 2>nul
else
clean:
	$(RM) *.o $(PROJ_NAME) $(UNITY_OBJ)
	$(RM) ../test/test_hash ../test/test_quadra ../test/test_grafo ../test/test_dijkstra
	$(RM) ../test/test_componentesConexos ../test/test_mst ../test/test_svg
	$(RM) ../test/test_parserGeo ../test/test_parserVia ../test/test_parserQry
endif

.PHONY: ted tstall clean \
        test_hash test_quadra test_grafo test_dijkstra \
        test_componentesConexos test_mst test_svg \
        test_parserGeo test_parserVia test_parserQry