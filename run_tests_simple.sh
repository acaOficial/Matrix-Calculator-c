#!/bin/bash

# Script simple de pruebas con validación básica

EXECUTABLE="./build/simple_calculator_v13"
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN}  CALCULATOR TEST - WITH VALIDATION${NC}"
echo -e "${CYAN}========================================${NC}"
echo ""

if [ ! -f "$EXECUTABLE" ]; then
    echo -e "${RED}ERROR: Ejecutable no encontrado${NC}"
    exit 1
fi

# Función para ejecutar una prueba
run_test() {
    local input="$1"
    local expected="$2"
    local description="$3"
    
    # Crear archivo temporal con la entrada
    echo -e "$input\nquit" > /tmp/calc_test_input.txt
    
    # Ejecutar y capturar salida completa
    full_output=$("$EXECUTABLE" < /tmp/calc_test_input.txt 2>&1)
    
    # Filtrar solo las líneas que empiezan con "= " después del prompt ">"
    # Ignorar las del mensaje de ayuda inicial
    output=$(echo "$full_output" | grep "^> = " | tail -1 | sed 's/^> = //')
    
    # Si no encontramos salida en una línea, buscar resultados multilínea (matrices)
    if [ -z "$output" ]; then
        # Buscar bloques que empiezan con "> =" (matrices multilínea)
        output=$(echo "$full_output" | awk '/^> =/{flag=1} flag; /^>$/{if(flag) exit}' | tail -20 | head -10 | grep -v "^>$" | sed 's/^> = //' | tr -d '\n' | head -c 100)
    fi
    
    if [ -n "$output" ]; then
        result="$output"
        
        # Comparar resultado (permitiendo pequeñas diferencias y usando grep -F para literales)
        if echo "$result" | grep -F "$expected" > /dev/null 2>&1; then
            echo -e "${GREEN}[OK]${NC}   $description"
            echo "       Input: $input => $result" | head -c 100
            return 0
        else
            echo -e "${RED}[FAIL]${NC} $description"
            echo "       Input: $input"
            echo -e "${YELLOW}       Esperado: $expected, Obtenido: $result${NC}" | head -c 150
            return 1
        fi
    else
        echo -e "${YELLOW}[WARN]${NC} $description (sin resultado)"
        return 1
    fi
}

echo -e "${YELLOW}Ejecutando pruebas...${NC}"
echo ""

passed=0
failed=0

echo -e "${CYAN}=== ARITMÉTICA BÁSICA ===${NC}"
if run_test "2 + 3;" "5" "Suma básica"; then ((passed++)); else ((failed++)); fi
if run_test "10 - 4;" "6" "Resta básica"; then ((passed++)); else ((failed++)); fi
if run_test "5 * 6;" "30" "Multiplicación"; then ((passed++)); else ((failed++)); fi
if run_test "20 / 4;" "5" "División"; then ((passed++)); else ((failed++)); fi
if run_test "17 % 5;" "2" "Módulo"; then ((passed++)); else ((failed++)); fi
if run_test "-8;" "-8" "Negativo"; then ((passed++)); else ((failed++)); fi
if run_test "+15;" "15" "Positivo"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== PRECEDENCIA Y PARÉNTESIS ===${NC}"
if run_test "2 + 3 * 4;" "14" "Precedencia básica"; then ((passed++)); else ((failed++)); fi
if run_test "(2 + 3) * 4;" "20" "Paréntesis simples"; then ((passed++)); else ((failed++)); fi
if run_test "10 - 3 - 2;" "5" "Asociatividad izquierda"; then ((passed++)); else ((failed++)); fi
if run_test "2 * 3 + 4 * 5;" "26" "Precedencia múltiple"; then ((passed++)); else ((failed++)); fi
if run_test "(2 + 3) * (4 + 5);" "45" "Paréntesis múltiples"; then ((passed++)); else ((failed++)); fi
if run_test "((2 + 3) * 4) + 5;" "25" "Paréntesis anidados"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== VARIABLES ===${NC}"
if run_test "x = 10;\nx;" "10" "Asignación simple"; then ((passed++)); else ((failed++)); fi
if run_test "x = 5;\ny = 3;\nx + y;" "8" "Suma de variables"; then ((passed++)); else ((failed++)); fi
if run_test "x = 10;\ny = 20;\nx * y;" "200" "Multiplicación de variables"; then ((passed++)); else ((failed++)); fi
if run_test "a = 5;\nb = a * 2;\nb;" "10" "Variable con expresión"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== CONSTANTES ===${NC}"
if run_test "const pi = 3.14159;\npi;" "3.14159" "Definición de constante"; then ((passed++)); else ((failed++)); fi
if run_test "const e = 2.71828;\ne * 2;" "5.43656" "Uso de constante"; then ((passed++)); else ((failed++)); fi
if run_test "const pi = 3.14159;\nconst r = 5;\npi * r * r;" "78.539" "Múltiples constantes"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== FUNCIONES MATEMÁTICAS ===${NC}"
if run_test "sin(0);" "0" "Seno de 0"; then ((passed++)); else ((failed++)); fi
if run_test "cos(0);" "1" "Coseno de 0"; then ((passed++)); else ((failed++)); fi
if run_test "tan(0);" "0" "Tangente de 0"; then ((passed++)); else ((failed++)); fi
if run_test "exp(0);" "1" "Exponencial de 0"; then ((passed++)); else ((failed++)); fi
if run_test "ln(1);" "0" "Logaritmo natural de 1"; then ((passed++)); else ((failed++)); fi
if run_test "log10(10);" "1" "Logaritmo base 10"; then ((passed++)); else ((failed++)); fi
if run_test "log10(100);" "2" "Log10 de 100"; then ((passed++)); else ((failed++)); fi
if run_test "log2(8);" "3" "Log2 de 8"; then ((passed++)); else ((failed++)); fi
if run_test "log2(16);" "4" "Log2 de 16"; then ((passed++)); else ((failed++)); fi
if run_test "pow(2, 3);" "8" "Potencia 2^3"; then ((passed++)); else ((failed++)); fi
if run_test "pow(2, 8);" "256" "Potencia 2^8"; then ((passed++)); else ((failed++)); fi
if run_test "pow(3, 3);" "27" "Potencia 3^3"; then ((passed++)); else ((failed++)); fi
if run_test "pow(10, 0);" "1" "Potencia x^0"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== VECTORES ===${NC}"
if run_test "v = {1, 2, 3};\nv;" "{1" "Vector simple"; then ((passed++)); else ((failed++)); fi
if run_test "w = {10, 20, 30, 40};\nw;" "{10" "Vector 4 elementos"; then ((passed++)); else ((failed++)); fi
if run_test "{};" "{}" "Vector vacío"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== MATRICES ===${NC}"
if run_test "m = {{1, 2}, {3, 4}};\nm;" "{1" "Matriz 2x2"; then ((passed++)); else ((failed++)); fi
if run_test "id = {{1, 0}, {0, 1}};\nid;" "{1" "Matriz identidad 2x2"; then ((passed++)); else ((failed++)); fi
if run_test "a = {{1, 2, 3}, {4, 5, 6}};\na;" "{1" "Matriz 2x3"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== OPERADOR TRANSPUESTA ===${NC}"
if run_test "v = {1, 2, 3};\n~v;" "{1" "Transpuesta de vector"; then ((passed++)); else ((failed++)); fi
if run_test "m = {{1, 2}, {3, 4}};\n~m;" "{1" "Transpuesta de matriz"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== FUNCIONES DEFINIDAS - BÁSICAS ===${NC}"
if run_test "doble(x) = x * 2;\ndoble(5);" "10" "Función simple"; then ((passed++)); else ((failed++)); fi
if run_test "triple(x) = x * 3;\ntriple(7);" "21" "Función triple"; then ((passed++)); else ((failed++)); fi
if run_test "cuadrado(x) = x * x;\ncuadrado(4);" "16" "Función cuadrado"; then ((passed++)); else ((failed++)); fi
if run_test "cuadrado(x) = x * x;\ncuadrado(9);" "81" "Cuadrado de 9"; then ((passed++)); else ((failed++)); fi
if run_test "cubo(x) = x * x * x;\ncubo(3);" "27" "Función cubo"; then ((passed++)); else ((failed++)); fi
if run_test "mitad(x) = x / 2;\nmitad(10);" "5" "Función mitad"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== FUNCIONES CON 2 PARÁMETROS ===${NC}"
if run_test "suma(a, b) = a + b;\nsuma(10, 20);" "30" "Suma de 2 números"; then ((passed++)); else ((failed++)); fi
if run_test "suma(a, b) = a + b;\nsuma(7, 13);" "20" "Suma 7+13"; then ((passed++)); else ((failed++)); fi
if run_test "resta(a, b) = a - b;\nresta(100, 25);" "75" "Resta"; then ((passed++)); else ((failed++)); fi
if run_test "producto(x, y) = x * y;\nproducto(6, 7);" "42" "Producto"; then ((passed++)); else ((failed++)); fi
if run_test "division(a, b) = a / b;\ndivision(100, 4);" "25" "División"; then ((passed++)); else ((failed++)); fi
if run_test "promedio(x, y) = (x + y) / 2;\npromedio(10, 30);" "20" "Promedio"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== FUNCIONES CON 3+ PARÁMETROS ===${NC}"
if run_test "suma3(a, b, c) = a + b + c;\nsuma3(1, 2, 3);" "6" "Suma de 3 números"; then ((passed++)); else ((failed++)); fi
if run_test "suma3(a, b, c) = a + b + c;\nsuma3(10, 20, 30);" "60" "Suma 10+20+30"; then ((passed++)); else ((failed++)); fi
if run_test "vol_caja(x, y, z) = x * y * z;\nvol_caja(2, 3, 4);" "24" "Volumen de caja"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== FUNCIONES CON NOMBRES DESCRIPTIVOS ===${NC}"
if run_test "area_rectangulo(b, h) = b * h;\narea_rectangulo(5, 8);" "40" "Área rectángulo"; then ((passed++)); else ((failed++)); fi
if run_test "area_triangulo(b, h) = b * h / 2;\narea_triangulo(6, 4);" "12" "Área triángulo"; then ((passed++)); else ((failed++)); fi
if run_test "perimetro_cuadrado(lado) = lado * 4;\nperimetro_cuadrado(5);" "20" "Perímetro cuadrado"; then ((passed++)); else ((failed++)); fi
if run_test "volumen_cubo(lado) = lado * lado * lado;\nvolumen_cubo(3);" "27" "Volumen cubo"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== FUNCIONES CON CONSTANTES ===${NC}"
if run_test "const pi = 3.14159;\narea_circulo(r) = pi * r * r;\narea_circulo(2);" "12.56" "Área círculo r=2"; then ((passed++)); else ((failed++)); fi
if run_test "const pi = 3.14159;\narea_circulo(r) = pi * r * r;\narea_circulo(5);" "78.539" "Área círculo r=5"; then ((passed++)); else ((failed++)); fi
if run_test "const pi = 3.14159;\ncircunferencia(r) = 2 * pi * r;\ncircunferencia(3);" "18.849" "Circunferencia"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== FUNCIONES CON FUNCIONES MATEMÁTICAS ===${NC}"
if run_test "raiz(x) = pow(x, 0.5);\nraiz(16);" "4" "Raíz cuadrada de 16"; then ((passed++)); else ((failed++)); fi
if run_test "raiz(x) = pow(x, 0.5);\nraiz(25);" "5" "Raíz cuadrada de 25"; then ((passed++)); else ((failed++)); fi
if run_test "raiz(x) = pow(x, 0.5);\nraiz(100);" "10" "Raíz cuadrada de 100"; then ((passed++)); else ((failed++)); fi
if run_test "hipotenusa(a, b) = pow(a*a + b*b, 0.5);\nhipotenusa(3, 4);" "5" "Hipotenusa 3-4-5"; then ((passed++)); else ((failed++)); fi
if run_test "hipotenusa(a, b) = pow(a*a + b*b, 0.5);\nhipotenusa(5, 12);" "13" "Hipotenusa 5-12-13"; then ((passed++)); else ((failed++)); fi
if run_test "seno_doble(x) = sin(2 * x);\nseno_doble(0);" "0" "Seno doble de 0"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== COMPOSICIÓN DE FUNCIONES ===${NC}"
if run_test "doble(x) = x * 2;\ncuadrado(x) = x * x;\ndoble_cuadrado(n) = doble(cuadrado(n));\ndoble_cuadrado(4);" "32" "Doble del cuadrado"; then ((passed++)); else ((failed++)); fi
if run_test "doble(x) = x * 2;\ncuadrado(x) = x * x;\ndoble_cuadrado(n) = doble(cuadrado(n));\ndoble_cuadrado(5);" "50" "Doble del cuadrado de 5"; then ((passed++)); else ((failed++)); fi
if run_test "doble(x) = x * 2;\ncuadrado(x) = x * x;\ncuadrado_doble(n) = cuadrado(doble(n));\ncuadrado_doble(3);" "36" "Cuadrado del doble"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== CADENAS DE FUNCIONES ===${NC}"
if run_test "f(x) = x + 1;\ng(x) = f(x) * 2;\nh(x) = g(x) - 5;\nh(10);" "17" "Cadena f→g→h"; then ((passed++)); else ((failed++)); fi
if run_test "f(x) = x + 1;\ng(x) = f(x) * 2;\ng(5);" "12" "f→g con 5"; then ((passed++)); else ((failed++)); fi
if run_test "inc(x) = x + 1;\ninc2(x) = inc(inc(x));\ninc2(10);" "12" "Incremento doble"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== FUNCIONES CON MÚLTIPLES LLAMADAS ===${NC}"
if run_test "doble(x) = x * 2;\ntriple(x) = x * 3;\nsuma_ambos(n) = doble(n) + triple(n);\nsuma_ambos(4);" "20" "Suma doble+triple"; then ((passed++)); else ((failed++)); fi
if run_test "cuadrado(x) = x * x;\ncubo(x) = x * x * x;\ndiferencia(n) = cubo(n) - cuadrado(n);\ndiferencia(3);" "18" "Cubo - cuadrado"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== FUNCIONES CON NOMBRES CON GUIONES BAJOS ===${NC}"
if run_test "mi_funcion(x) = x * 2;\nmi_funcion(7);" "14" "Nombre con guión bajo"; then ((passed++)); else ((failed++)); fi
if run_test "funcion_larga_nombre(x) = x + 10;\nfuncion_larga_nombre(5);" "15" "Nombre largo"; then ((passed++)); else ((failed++)); fi
if run_test "_privada(x) = x * 3;\n_privada(4);" "12" "Nombre empieza con _"; then ((passed++)); else ((failed++)); fi
if run_test "calcular_area_total(a, b) = a * b;\ncalcular_area_total(6, 8);" "48" "Nombre muy descriptivo"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== FUNCIONES COMPLEJAS ===${NC}"
if run_test "discriminante(a, b, c) = b*b - 4*a*c;\ndiscriminante(1, 5, 6);" "1" "Discriminante positivo"; then ((passed++)); else ((failed++)); fi
if run_test "discriminante(a, b, c) = b*b - 4*a*c;\ndiscriminante(1, 2, 1);" "0" "Discriminante cero"; then ((passed++)); else ((failed++)); fi
if run_test "const g = 9.81;\ncaida(t) = 0.5 * g * t * t;\ncaida(2);" "19.6" "Caída libre"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== CONVERSIONES ===${NC}"
if run_test "c_a_f(c) = c * 9 / 5 + 32;\nc_a_f(0);" "32" "0°C a Fahrenheit"; then ((passed++)); else ((failed++)); fi
if run_test "c_a_f(c) = c * 9 / 5 + 32;\nc_a_f(100);" "212" "100°C a Fahrenheit"; then ((passed++)); else ((failed++)); fi
if run_test "f_a_c(f) = (f - 32) * 5 / 9;\nf_a_c(32);" "0" "32°F a Celsius"; then ((passed++)); else ((failed++)); fi
if run_test "f_a_c(f) = (f - 32) * 5 / 9;\nf_a_c(212);" "100" "212°F a Celsius"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== EXPRESIONES COMPLEJAS ===${NC}"
if run_test "x = 5;\ny = 10;\nz = 15;\nx + y * z;" "155" "Variables con precedencia"; then ((passed++)); else ((failed++)); fi
if run_test "a = 2;\nb = 3;\nc = 4;\n(a + b) * c - a;" "18" "Expresión compleja"; then ((passed++)); else ((failed++)); fi
if run_test "const pi = 3.14159;\nr = 5;\nh = 10;\npi * r * r * h;" "785.39" "Volumen cilindro"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}=== CASOS ESPECIALES ===${NC}"
if run_test "0 * 100;" "0" "Multiplicación por 0"; then ((passed++)); else ((failed++)); fi
if run_test "100 * 1;" "100" "Multiplicación por 1"; then ((passed++)); else ((failed++)); fi
if run_test "0 + 50;" "50" "Suma con 0"; then ((passed++)); else ((failed++)); fi
if run_test "50 - 0;" "50" "Resta de 0"; then ((passed++)); else ((failed++)); fi
if run_test "pow(0, 5);" "0" "0 elevado a n"; then ((passed++)); else ((failed++)); fi
if run_test "pow(5, 1);" "5" "n elevado a 1"; then ((passed++)); else ((failed++)); fi

echo ""
echo -e "${CYAN}========================================${NC}"
echo "Resultados:"
echo -e "  ${GREEN}Pasadas: $passed${NC}"
if [ $failed -eq 0 ]; then
    echo -e "  ${GREEN}Fallidas: $failed${NC}"
else
    echo -e "  ${RED}Fallidas: $failed${NC}"
fi
echo -e "${CYAN}========================================${NC}"
echo ""

if [ $failed -eq 0 ]; then
    echo -e "${GREEN}¡Todas las pruebas pasaron exitosamente! ✓${NC}"
    exit 0
else
    echo -e "${RED}Algunas pruebas fallaron. ✗${NC}"
    exit 1
fi

# Limpiar
rm -f /tmp/calc_test_input.txt
