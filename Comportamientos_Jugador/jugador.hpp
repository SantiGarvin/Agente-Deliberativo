#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>
#include <queue>
#include <vector>
#include <cmath>
#include <iostream>

using namespace std;

class ComportamientoJugador : public Comportamiento
{
public:
	struct Nodo;
	typedef vector<vector<Nodo>> Grafo;

	struct Celda;
	typedef vector<vector<Celda>> Mapa;

	enum Terreno
	{
		Bosque = 'B',
		Agua = 'A',
		Precipicio = 'P',
		SueloPiedra = 'S',
		SueloArenoso = 'T',
		Muro = 'M',
		Bikini = 'K',
		Zapatillas = 'D',
		Recarga = 'X',
		Desconocido = '?'
	};

	enum Entidad
	{
		Jugador = 'j',
		Sonambulo = 's',
		Aldeano = 'a',
		Lobo = 'l',
		SinEntidad = '_'
	};

	struct Celda
	{
		unsigned char terreno;
		unsigned char superficie;

		Celda()
			: terreno{Terreno::Desconocido},
			  superficie{Entidad::SinEntidad}
		{
		}

		Celda(unsigned char t)
			: terreno{static_cast<Terreno>(t)},
			  superficie{Entidad::SinEntidad}
		{
		}

		Celda(Terreno t, Entidad s)
			: terreno{t},
			  superficie{s}
		{
		}

		bool esTransitable() const
		{
			return (terreno != Terreno::Precipicio && terreno != Terreno::Muro && superficie != Entidad::Lobo && superficie != Entidad::Aldeano && superficie != Entidad::Sonambulo);
		}
	};

	//////////////////////////////////////////////////////////////////
	// Struct NODO													//
	//////////////////////////////////////////////////////////////////
	struct Nodo
	{
		vector<Action> acciones; // Acciones posibles desde el nodo
		vector<Nodo *> vecinos;  // Punteros a los nodos vecinos

		Nodo *padre; // Puntero al nodo padre

		ubicacion pos; // Posición en el mapa y orientación
		Celda celda;   // Información de la celda
		bool visitado; // Indica si el nodo ha sido visitado

		vector<int> costos; // Costos de las acciones
		int costoAcumulado; // Costo acumulado
		int costoEstimado;	// Costo estimado hasta la meta (para A*)
		int heuristica;		// Heurística para el cálculo del costo estimado

		Nodo()
			: acciones{},
			  vecinos{},
			  padre{nullptr},
			  celda{Terreno::Desconocido, Entidad::SinEntidad},
			  pos{0, 0, Orientacion::norte},
			  visitado{false},
			  costoAcumulado{0},
			  costoEstimado{0},
			  heuristica{0}
		{
		}

		Nodo(int f, int c, Orientacion o = Orientacion::norte)
			: acciones{},
			  vecinos{},
			  padre{nullptr},
			  celda{Terreno::Desconocido, Entidad::SinEntidad},
			  pos{f, c, o},
			  visitado{false},
			  costoAcumulado{0},
			  costoEstimado{0},
			  heuristica{0}
		{
		}

		Nodo(int f, int c, Terreno t = Terreno::Desconocido, Entidad s = Entidad::SinEntidad)
			: acciones{},
			  vecinos{},
			  padre{nullptr},
			  celda{t, s},
			  pos{f, c, Orientacion::norte},
			  visitado{false},
			  costoAcumulado{0},
			  costoEstimado{0},
			  heuristica{0}
		{
		}

		// Comparador para ordenar nodos en priority_queue
		bool operator<(const Nodo &other) const
		{
			return (costoAcumulado + costoEstimado) > (other.costoAcumulado + other.costoEstimado);
		}

		void conectar(Nodo *otro_nodo, Action accion)
		{
			vecinos.push_back(otro_nodo);
			acciones.push_back(accion);
		}

		void desconectar(Nodo *otro_nodo)
		{
			for (size_t i = 0; i < vecinos.size(); ++i)
			{
				if (vecinos[i] == otro_nodo)
				{
					vecinos.erase(vecinos.begin() + i);
					acciones.erase(acciones.begin() + i);
					break;
				}
			}
		}

		int calcularHeuristica(Nodo *objetivo)
		{
			// Ejemplo de heurística: distancia de Manhattan
			int distancia_f = abs(pos.f - objetivo->pos.f);
			int distancia_c = abs(pos.c - objetivo->pos.c);
			heuristica = distancia_f + distancia_c;
			return heuristica;
		}

		bool esObjetivo(Nodo *objetivo)
		{
			return pos == objetivo->pos;
		}

		bool esTransitable()
		{
			return celda.esTransitable();
		}

		void imprimirInfo()
		{
			std::cout << "Posicion: (" << pos.f << ", " << pos.c << ")" << std::endl;
			std::cout << "Orientacion: " << static_cast<int>(pos.brujula) << std::endl;
			std::cout << "Terreno: " << static_cast<int>(celda.terreno) << std::endl;
			std::cout << "Superficie: " << static_cast<int>(celda.superficie) << std::endl;
			std::cout << "Costo acumulado: " << costoAcumulado << std::endl;
			std::cout << "Heuristica: " << heuristica << std::endl;
		}
	};

	struct Estado
	{
		int nivel;
		int bateria;
		bool colision;
		bool reset;
		ubicacion jugador;
		ubicacion sonambulo;
		bool tieneBikini;
		bool tieneZapatillas;
	};

	////////////////////////////////////////////////////////////////

	ComportamientoJugador(unsigned int size) : Comportamiento(size), grafo(size, vector<Nodo>(size)), mapaAux(2 * size, vector<Celda>(2 * size))
	{
		inicializaVariablesEstado();
		dimensionMapa = size;
	}

	ComportamientoJugador(std::vector<std::vector<unsigned char>> mapaR) : Comportamiento(mapaR), grafo(mapaR.size(), vector<Nodo>(mapaR.size())), mapaAux(2 * mapaR.size(), vector<Celda>(2 * mapaR.size()))
	{
		inicializaVariablesEstado();
		dimensionMapa = mapaR.size();
	}

	ComportamientoJugador(const ComportamientoJugador &comport) : Comportamiento(comport) {}
	~ComportamientoJugador() {}

	Action think(Sensores sensores);
	int interact(Action accion, int valor);

private:
	//////////////////////////////////////////////////////////////////
	// Variables de estado											//
	//////////////////////////////////////////////////////////////////

	Estado estadoActual;

	int tiempo;

	Action ultimaAccionJugador;
	Action ultimaAccionSonambulo;

	//////////////////////////////////////////////////////////////////
	// Planes														//
	//////////////////////////////////////////////////////////////////

	bool hayPlan;

	queue<Action> planBFS;
	priority_queue<Action> planAStar;
	priority_queue<Action> planDijkstra;

	//////////////////////////////////////////////////////////////////
	// Mapas - Ubicaciones											//
	//////////////////////////////////////////////////////////////////

	const int MAX_MAPA = 100;
	int dimensionMapa;

	Mapa mapaAux; // solo para nivel 4
	Grafo grafo;
	list<Nodo *> ruta;

	vector<ubicacion> sonambulos;
	vector<ubicacion> aldeanos;
	vector<ubicacion> lobos;

	vector<unsigned char> vision;

	//////////////////////////////////////////////////////////////////
	// Funciones privadas											//
	//////////////////////////////////////////////////////////////////

	// INICIALIZACION
	void inicializaVariablesEstado();
	void inicializarGrafo(int tamanio);

	// ACTUALIZACIONES
	void actualizaEstado(const Sensores &sensores);
	void actualizaPosicionOrientacion(bool esJugador);
	void actualizaGrafo(const Mapa &mapa);
  void actualizaMapaAux();
	void actualizaMapaVisionJugador(const Sensores &sensores);

	// FUNCIONES AUXILIARES
	int calcularCostoBateria(Action accion, unsigned char tipoCasilla);
	list<Action> busquedaAnchuraJugador();
	list<Action> busquedaAnchuraSonambulo();
	list<Action> encuentraCaminoDijkstraJugador();
	list<Action> encuentraCaminoAStarSonambulo();
	list<Action> maximizarPuntuacion();

	ubicacion siguienteCasilla(const ubicacion &pos);

	void conectarNodos(int fila1, int columna1, int fila2, int columna2);

	// Action eligeAccionPlan();

	Action accionEntreNodos(Nodo *origen, Nodo *destino);

	bool esObjetivo(const Nodo &objetivo);
	Orientacion orientacionEntreNodos(const Nodo &origen, const Nodo &destino);

	void anularMatriz(vector<vector<unsigned char>> &matriz);
	void visualizaPlan(const list<Action> &plan);

	ubicacion nextCasilla(const ubicacion &pos);

	// DEBUG
	char orientacionASimbolo(Orientacion o);
	void imprimirGrafo();

  Terreno charToTerreno(unsigned char c);
  Entidad charToEntidad(unsigned char c);
};

#endif
