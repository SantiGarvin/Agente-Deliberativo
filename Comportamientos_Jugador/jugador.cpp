#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>

using Nodo = ComportamientoJugador::Nodo;
using Grafo = ComportamientoJugador::Grafo;
using Celda = ComportamientoJugador::Celda;
using Mapa = ComportamientoJugador::Mapa;
using Terreno = ComportamientoJugador::Terreno;
using Entidad = ComportamientoJugador::Entidad;
using Estado = ComportamientoJugador::Estado;
using Objeto = ComportamientoJugador::Objeto;

// Este es el método principal que se piden en la practica.
// Tiene como entrada la información de los sensores y devuelve la acción a realizar.
// Para ver los distintos sensores mirar fichero "comportamiento.hpp"
Action ComportamientoJugador::think(Sensores sensores)
{
	Action accion = actIDLE;

	actualizaEstado(sensores);
	actualizaVisionJugador(estadoActual);
	actualizaPosicionOrientacion();

	if (sensores.nivel != 4)
	{
		// Actualizar información de los sensores
		actualizarMapaCeldas();
		actualizaGrafo(mapaAux);

		debug(false);

		if (!hayPlan)
		{
			cout << "Calculando un nuevo plan\n";

			ubicacion destino = {sensores.destinoF, sensores.destinoC};

			switch (sensores.nivel)
			{
			case 0:
				plan = busquedaAnchuraJugador(estadoActual, destino);
				break;
			case 1:
				plan = busquedaAnchuraSonambulo(estadoActual, destino);
				break;
			case 2:
				plan = busquedaDijkstraJugador(estadoActual, destino);
				break;
			// case 3:
			// 	plan = encuentraCaminoAStarSonambulo();
			// 	break;
			// case 4:
			// 	plan = maximizarPuntuacion();
			// 	break;
			}
			if (plan.size() > 0)
			{
				cout << "___PLAN___" << endl;
				for (Action a : plan)
					cout << toString(a) << " ";
				cout << endl;

				cout << "Dibujando plan\n";
				visualizaPlan(plan);
				hayPlan = true;
			}
		}
		if (hayPlan && plan.size() > 0)
		{
			cout << "Ejecutando la siguiente accion del plan\n";
			accion = plan.front();
			plan.pop_front();
		}
		if (plan.size() == 0)
		{
			cout << "Se completo el plan\n";
			hayPlan = false;
		}
	}
	else
	{
		// Verificar si es el inicio de la simulación, si hay un reinicio o si es la primera iteración
		if (sensores.reset || primeraIteracion)
		{
			primeraIteracion = false; // Cambiar el valor de primeraIteracion a false para que no entre en el if en futuras iteraciones
			return actWHEREIS;
		}

		actualizaMapaResultVisionJugador(sensores);
		actualizaGrafo(mapaAux);

		// ------------------
		// COMPLETAR NIVEL 4
		// ------------------
	}

	switch (accion)
	{
	case actFORWARD:
	case actTURN_L:
	case actTURN_R:
		ultimaAccionJugador = accion;
		break;
	case actSON_FORWARD:
	case actSON_TURN_SL:
	case actSON_TURN_SR:
		ultimaAccionSonambulo = accion;
		break;
	}

	return accion;
}

void ComportamientoJugador::actualizaEstado(const Sensores &sensores)
{
	nivel = sensores.nivel;
	bateria = sensores.bateria;

	if (sensores.nivel != 4)
	{
		// Actualizar posición del jugador
		estadoActual.jugador.f = sensores.posF;
		estadoActual.jugador.c = sensores.posC;
		estadoActual.jugador.brujula = sensores.sentido;

		// Actualizar posición del sonámbulo
		estadoActual.sonambulo.f = sensores.SONposF;
		estadoActual.sonambulo.c = sensores.SONposC;
		estadoActual.sonambulo.brujula = sensores.SONsentido;
	}

	int f = estadoActual.jugador.f;
	int c = estadoActual.jugador.c;

	if (mapaAux[f][c].terreno == Terreno::Bikini && !tieneBikini && !tieneZapatillas)
		tieneBikini = true;
	else
		tieneBikini = false;

	if (mapaAux[f][c].terreno == Terreno::Zapatillas && !tieneZapatillas && !tieneBikini)
		tieneZapatillas = true;
	else
		tieneZapatillas = false;

	tiempo = sensores.tiempo;
}

void ComportamientoJugador::actualizaPosicionOrientacion()
{
	switch (ultimaAccionJugador)
	{
	case actFORWARD:
		switch (estadoActual.jugador.brujula)
		{
		case norte:
			estadoActual.jugador.f--;
			break;
		case noreste:
			estadoActual.jugador.f--;
			estadoActual.jugador.c++;
			break;
		case este:
			estadoActual.jugador.c++;
			break;
		case sureste:
			estadoActual.jugador.f++;
			estadoActual.jugador.c++;
			break;
		case sur:
			estadoActual.jugador.f++;
			break;
		case suroeste:
			estadoActual.jugador.f++;
			estadoActual.jugador.c--;
			break;
		case oeste:
			estadoActual.jugador.c--;
			break;
		case noroeste:
			estadoActual.jugador.f--;
			estadoActual.jugador.c--;
			break;
		}
		break;
	case actTURN_R: // giro a la izquierda 90 grados
		estadoActual.jugador.brujula = static_cast<Orientacion>((estadoActual.jugador.brujula + 2) % 8);
		break;
	case actTURN_L: // giro a la derecha 90 grados
		estadoActual.jugador.brujula = static_cast<Orientacion>((estadoActual.jugador.brujula + 6) % 8);
		break;
	default:
		break;
	}

	switch (ultimaAccionSonambulo)
	{
	case actSON_FORWARD:
		switch (estadoActual.sonambulo.brujula)
		{

		case norte:
			estadoActual.sonambulo.f--;
			break;
		case noreste:
			estadoActual.sonambulo.f--;
			estadoActual.sonambulo.c++;
			break;
		case este:
			estadoActual.sonambulo.c++;
			break;
		case sureste:
			estadoActual.sonambulo.f++;
			estadoActual.sonambulo.c++;
			break;
		case sur:
			estadoActual.sonambulo.f++;
			break;
		case suroeste:
			estadoActual.sonambulo.f++;
			estadoActual.sonambulo.c--;
			break;
		case oeste:
			estadoActual.sonambulo.c--;
			break;
		case noroeste:
			estadoActual.sonambulo.f--;
			estadoActual.sonambulo.c--;
			break;
		}
		break;
	case actTURN_SR: // giro a la derecha 45 grados
		estadoActual.sonambulo.brujula = static_cast<Orientacion>((estadoActual.jugador.brujula + 1) % 8);
		break;
	case actTURN_SL: // giro a la izquierda 45 grados
		estadoActual.sonambulo.brujula = static_cast<Orientacion>((estadoActual.jugador.brujula + 7) % 8);
		break;
	default:
		break;
	}
}

void ComportamientoJugador::actualizaMapaAux()
{
	for (int i = 0; i < mapaResultado.size(); ++i)
	{
		for (int j = 0; j < mapaResultado[0].size(); ++j)
		{
			mapaAux[i][j].terreno = charATerreno(mapaResultado[i][j]);
			mapaAux[i][j].superficie = charAEntidad(mapaEntidades[i][j]);
		}
	}
}

void ComportamientoJugador::actualizaGrafo(const Mapa &mapaAux)
{
	int filas = grafo.size();
	int columnas = grafo[0].size();

	for (int f = 0; f < filas; ++f)
	{
		for (int c = 0; c < columnas; ++c)
		{
			grafo[f][c].celda.terreno = charATerreno(mapaAux[f][c].terreno);
			grafo[f][c].celda.superficie = charAEntidad(mapaAux[f][c].superficie);
		}
	}
}

Terreno ComportamientoJugador::charATerreno(unsigned char c) const
{
	switch (c)
	{
	case 'B':
		return Terreno::Bosque;
	case 'A':
		return Terreno::Agua;
	case 'P':
		return Terreno::Precipicio;
	case 'S':
		return Terreno::SueloPiedra;
	case 'T':
		return Terreno::SueloArenoso;
	case 'M':
		return Terreno::Muro;
	case 'K':
		return Terreno::Bikini;
	case 'D':
		return Terreno::Zapatillas;
	case 'X':
		return Terreno::Recarga;
	default:
		return Terreno::Desconocido;
	}
}

Entidad ComportamientoJugador::charAEntidad(unsigned char c) const
{
	switch (c)
	{
	case 'j':
		return Entidad::Jugador;
	case 's':
		return Entidad::Sonambulo;
	case 'a':
		return Entidad::Aldeano;
	case 'l':
		return Entidad::Lobo;
	default:
		return Entidad::SinEntidad;
	}
}

void ComportamientoJugador::inicializaVariablesEstado()
{
	nivel = -1;
	primeraIteracion = true;
	estadoActual.jugador.f = 99;
	estadoActual.jugador.c = 99;
	estadoActual.jugador.brujula = norte;
	estadoActual.sonambulo.f = 99;
	estadoActual.sonambulo.c = 99;
	estadoActual.sonambulo.brujula = norte;
	tieneBikini = false;
	tieneZapatillas = false;
	sonambuloEnVision = false;

	tiempo = 0;
	hayPlan = false;

	ultimaAccionJugador = actIDLE;
	ultimaAccionSonambulo = actIDLE;

	vision.resize(16);
}

void ComportamientoJugador::anularMapaConPlan()
{
	for (int i = 0; i < mapaConPlan.size(); ++i)
	{
		for (int j = 0; j < mapaConPlan.size(); ++j)
		{
			mapaConPlan[i][j] = 0;
		}
	}
}

void ComportamientoJugador::visualizaPlan(const list<Action> &plan)
{
	anularMapaConPlan();
	Estado cst = estadoActual;

	auto it = plan.begin();
	while (it != plan.end())
	{
		switch (*it)
		{
		case actFORWARD:
			cst.jugador = siguienteCasilla(cst.jugador);
			mapaConPlan[cst.jugador.f][cst.jugador.c] = 1;
			break;
		case actTURN_R:
			cst.jugador.brujula = static_cast<Orientacion>((cst.jugador.brujula + 2) % 8);
			break;
		case actTURN_L:
			cst.jugador.brujula = static_cast<Orientacion>((cst.jugador.brujula + 6) % 8);
			break;
		case actSON_FORWARD:
			cst.sonambulo = siguienteCasilla(cst.sonambulo);
			mapaConPlan[cst.sonambulo.f][cst.sonambulo.c] = 2;
			break;
		case actSON_TURN_SR:
			cst.sonambulo.brujula = static_cast<Orientacion>((cst.sonambulo.brujula + 1) % 8);
			break;
		case actSON_TURN_SL:
			cst.sonambulo.brujula = static_cast<Orientacion>((cst.sonambulo.brujula + 7) % 8);
			break;
		}
		it++;
	}
}

ubicacion ComportamientoJugador::siguienteCasilla(const ubicacion &pos)
{
	ubicacion next = pos;

	switch (pos.brujula)
	{
	case norte:
		next.f = pos.f - 1;
		break;
	case noreste:
		next.f = pos.f - 1;
		next.c = pos.c + 1;
		break;
	case este:
		next.c = pos.c + 1;
		break;
	case sureste:
		next.f = pos.f + 1;
		next.c = pos.c + 1;
		break;
	case sur:
		next.f = pos.f + 1;
		break;
	case suroeste:
		next.f = pos.f + 1;
		next.c = pos.c - 1;
		break;
	case oeste:
		next.c = pos.c - 1;
		break;
	case noroeste:
		next.f = pos.f - 1;
		next.c = pos.c - 1;
		break;
	}

	if (next.f < 0 || next.f >= mapaResultado.size() || next.c < 0 || next.c >= mapaResultado.size())
		return pos;
	else
		return next;
}

Estado ComportamientoJugador::aplicar(const Action &a, const Estado &st)
{
	Estado stResult = st;
	ubicacion sigUbicacion;

	switch (a)
	{
	case actFORWARD:
		sigUbicacion = siguienteCasilla(st.jugador);
		if (casillaTransitable(sigUbicacion) && !(sigUbicacion.f == st.sonambulo.f && sigUbicacion.c == st.sonambulo.c))
			stResult.jugador = sigUbicacion;
		break;
	case actTURN_L:
		stResult.jugador.brujula = static_cast<Orientacion>((stResult.jugador.brujula + 6) % 8);
		break;
	case actTURN_R:
		stResult.jugador.brujula = static_cast<Orientacion>((stResult.jugador.brujula + 2) % 8);
		break;
	case actSON_FORWARD:
		sigUbicacion = siguienteCasilla(st.sonambulo);
		if (casillaTransitable(sigUbicacion) && !(sigUbicacion.f == st.jugador.f && sigUbicacion.c == st.jugador.c))
			stResult.sonambulo = sigUbicacion;
		break;
	case actSON_TURN_SL:
		stResult.sonambulo.brujula = static_cast<Orientacion>((stResult.sonambulo.brujula + 7) % 8);
		break;
	case actSON_TURN_SR:
		stResult.sonambulo.brujula = static_cast<Orientacion>((stResult.sonambulo.brujula + 1) % 8);
		break;
	}
	return stResult;
}

bool ComportamientoJugador::casillaTransitable(const ubicacion &pos)
{
	return mapaCeldas[pos.f][pos.c].esTransitable();
}

list<Action> ComportamientoJugador::busquedaAnchuraJugador(const Estado &origen, const ubicacion &destino)
{
	Nodo nodoActual(this);
	list<Nodo> frontera;
	set<Nodo> explorados;
	list<Action> plan;

	// Inicializar el nodo actual
	nodoActual.estado = origen;

	// Comprobar si el origen es el destino
	bool SolucionEncontrada = (nodoActual.estado.jugador.f == destino.f && nodoActual.estado.jugador.c == destino.c);

	// Si no es el destino, generar los hijos
	frontera.push_back(nodoActual);

	// Mientras haya nodos en la frontera y no se haya encontrado la solución
	while (!frontera.empty() && !SolucionEncontrada)
	{
		// Extraer el primer nodo de la frontera
		frontera.pop_front();
		explorados.insert(nodoActual);

		// Gererar hijo actFORWARD
		Nodo hijoForward = nodoActual;
		hijoForward.estado = aplicar(actFORWARD, nodoActual.estado);

		if (hijoForward.estado.jugador.f == destino.f && hijoForward.estado.jugador.c == destino.c)
		{
			hijoForward.secuencia.push_back(actFORWARD);
			nodoActual = hijoForward;
			SolucionEncontrada = true;
		}
		else if (explorados.find(hijoForward) == explorados.end())
		{
			hijoForward.secuencia.push_back(actFORWARD);
			frontera.push_back(hijoForward);
		}

		if (!SolucionEncontrada)
		{ // Si no es el destino, generar los hijos

			// Generar hijo actTURN_L
			Nodo hijoTurnL = nodoActual;
			hijoTurnL.estado = aplicar(actTURN_L, nodoActual.estado);

			if (explorados.find(hijoTurnL) == explorados.end())
			{
				hijoTurnL.secuencia.push_back(actTURN_L);
				frontera.push_back(hijoTurnL);
			}

			// Generar hijo actTURN_R
			Nodo hijoTurnR = nodoActual;
			hijoTurnR.estado = aplicar(actTURN_R, nodoActual.estado);

			if (explorados.find(hijoTurnR) == explorados.end())
			{
				hijoTurnR.secuencia.push_back(actTURN_R);
				frontera.push_back(hijoTurnR);
			}
		}

		if (!SolucionEncontrada && !frontera.empty())
		{
			nodoActual = frontera.front();

			while (!frontera.empty() && explorados.find(nodoActual) != explorados.end())
			{
				frontera.pop_front();

				if (!frontera.empty())
					nodoActual = frontera.front();
			}
		}
	}

	if (SolucionEncontrada)
	{
		plan = nodoActual.secuencia;

		return plan;
	}

	// Si no se encuentra un plan, devolvemos una lista vacía
	return list<Action>();
}

list<Action> ComportamientoJugador::busquedaAnchuraSonambulo(const Estado &origen, const ubicacion &destino)
{
	Nodo n(this);
	list<Nodo> frontera;
	set<Nodo> explorados;

	n.estado = origen;
	n.accion = actIDLE;

	frontera.push_back(n);

	if (esDestinoSonambulo(n.estado, destino))
		return list<Action>();

	while (!frontera.empty())
	{
		frontera.pop_front();
		explorados.insert(n);

		actualizaVisionJugador(n.estado);

		list<Action> acciones;
		if (sonambuloEnVision)
		{
			acciones = {actSON_TURN_SL, actSON_FORWARD, actSON_TURN_SR};
		}
		else
		{
			acciones = {actTURN_L, actFORWARD, actTURN_R};
		}

		for (Action a : acciones)
		{
			procesarAccion(&n, a, frontera, explorados);

			if (esDestinoSonambulo(n.estado, destino))
			{
				return n.secuencia;
			}
		}

		while (!frontera.empty() && explorados.find(frontera.front()) != explorados.end())
		{
			frontera.pop_front();
		}

		if (!frontera.empty())
			n = frontera.front();
	}

	return list<Action>();
}

void ComportamientoJugador::procesarAccion(const Nodo *n, const Action &a, list<Nodo> &frontera, set<Nodo> &explorados)
{
	Nodo hijo = *n;
	hijo.accion = a;
	hijo.estado = aplicar(a, n->estado);
	hijo.secuencia = n->secuencia; // Copia la secuencia del padre
	hijo.secuencia.push_back(a);   // Agrega la acción actual al final de la secuencia

	if (explorados.find(hijo) == explorados.end())
	{
		frontera.push_back(hijo); // Añade el hijo a la frontera
	}
}

// list<Action> ComportamientoJugador::busquedaDijkstraJugador(const Estado &origen, const ubicacion &destino)
// {
// 	priority_queue<Nodo, vector<Nodo>, greater<Nodo>> cola;
// 	unordered_map<string, Nodo, EstadoHasherJugador> visitados;

// 	// Crear nodo inicial con la orientación actual del jugador
// 	Estado estadoInicial = origen;
// 	Nodo nodoInicial(this, actIDLE, estadoInicial, getMapaCeldas()[estadoInicial.jugador.f][estadoInicial.jugador.c], {}, 0, 0);
// 	cola.push(nodoInicial);

// 	int contador = 0;

// 	while (!cola.empty())
// 	{
// 		Nodo actual = cola.top();
// 		cola.pop();

// 		string claveActual = generarClave(actual.estado);

// 		// Si ya se visitó el estado, continuar con el siguiente nodo
// 		if (visitados.find(claveActual) != visitados.end())
// 		{
// 			continue;
// 		}

// 		visitados[claveActual] = actual;

// 		if (actual.estado.jugador.f == destino.f && actual.estado.jugador.c == destino.c)
// 		{
// 			cout << "ITERACIONES: " << contador << endl;
// 			cout << "ABIERTOS: " << cola.size() << endl;
// 			cout << "CERRADOS: " << visitados.size() << endl;
// 			return actual.secuencia;
// 		}

// 		vector<Action> accionesPosibles = {actFORWARD, actTURN_L, actTURN_R};

// 		for (const auto &accion : accionesPosibles)
// 		{
// 			Nodo sucesor = aplicarAccionCosto(actual, accion);
// 			string claveSucesor = generarClave(sucesor.estado);

// 			if (sucesor.valido && visitados.find(claveSucesor) == visitados.end())
// 			{
// 				cola.push(sucesor);
// 			}
// 		}
// 		contador++;
// 	}

// 	// Si no se encuentra una solución, devolver una lista vacía
// 	return list<Action>();
// }

list<Action> ComportamientoJugador::busquedaDijkstraJugador(const Estado &origen, const ubicacion &destino)
{
	priority_queue<Nodo, vector<Nodo>, less<Nodo>> cola;
	unordered_map<string, Nodo, EstadoHasherJugador> visitados;

	// Crear nodo inicial con la orientación actual del jugador
	Estado estadoInicial = origen;
	Nodo nodoInicial(this, actIDLE, estadoInicial, getMapaCeldas()[estadoInicial.jugador.f][estadoInicial.jugador.c], {}, 0, 0);
	cola.push(nodoInicial);

	int contador = 0;

	while (!cola.empty())
	{
		Nodo actual = cola.top();
		cola.pop();

		string claveActual = generarClave(actual.estado);

		// Si ya se visitó el estado y el costo acumulado es menor o igual, continuar con el siguiente nodo
		auto it = visitados.find(claveActual);
		if (it != visitados.end() && it->second.costoAcumulado <= actual.costoAcumulado)
		{
			continue;
		}

		visitados[claveActual] = actual;

		if (actual.estado.jugador.f == destino.f && actual.estado.jugador.c == destino.c)
		{
			cout << "ITERACIONES: " << contador << endl;
			cout << "ABIERTOS: " << cola.size() << endl;
			cout << "CERRADOS: " << visitados.size() << endl;
			return actual.secuencia;
		}

		vector<Action> accionesPosibles = {actFORWARD, actTURN_L, actTURN_R};

		for (const auto &accion : accionesPosibles)
		{
			Nodo sucesor = aplicarAccionCosto(actual, accion);
			string claveSucesor = generarClave(sucesor.estado);

			if (sucesor.valido)
			{
				cola.push(sucesor);
			}
		}
		contador++;
	}

	// Si no se encuentra una solución, devolver una lista vacía
	return list<Action>();
}

string ComportamientoJugador::generarClave(const Estado &estado)
{
	stringstream clave;
	clave << estado.jugador.f << "," << estado.jugador.c << "," << estado.jugador.brujula << ","
		  << (estado.objetoJugador.zapatillas ? "1" : "0") << "," << (estado.objetoJugador.bikini ? "1" : "0") << "_"
		  << estado.sonambulo.f << "," << estado.sonambulo.c << "," << estado.sonambulo.brujula << ","
		  << (estado.objetoSonambulo.zapatillas ? "1" : "0") << "," << (estado.objetoSonambulo.bikini ? "1" : "0");
	return clave.str();
}

Nodo ComportamientoJugador::aplicarAccionCosto(const Nodo &nodo, const Action &accion)
{
	// Aplicamos la acción al estado actual del nodo.
	Estado estadoNuevo = aplicar(accion, nodo.estado);

	// Verificamos si la casilla resultante es transitable.
	ubicacion nuevaUbicacionJugador = estadoNuevo.jugador;
	if (!casillaTransitable(nuevaUbicacionJugador))
	{
		Nodo nodoInvalido = nodo;
		nodoInvalido.valido = false;
		return nodoInvalido;
	}

	Terreno objeto = charATerreno(getMapaCeldas()[nuevaUbicacionJugador.f][nuevaUbicacionJugador.c].terreno);
	if (objeto == Terreno::Zapatillas)
	{
		estadoNuevo.objetoJugador.zapatillas = true;
		estadoNuevo.objetoJugador.bikini = false;
	}
	else if (objeto == Terreno::Bikini)
	{
		estadoNuevo.objetoJugador.bikini = true;
		estadoNuevo.objetoJugador.zapatillas = false;
	}

	// Calculamos el costo de batería asociado con la acción.
	Celda celdaActual = mapaCeldas[nuevaUbicacionJugador.f][nuevaUbicacionJugador.c];
	int costoBateria = calcularCostoBateria(accion, celdaActual.terreno, estadoNuevo.objetoJugador);

	// Creamos un nuevo nodo con el estado actualizado y el costo de batería actualizado.
	Nodo nodoNuevo = nodo;
	nodoNuevo.estado = estadoNuevo;
	nodoNuevo.costoAcumulado += costoBateria;

	// Añadimos la acción a la secuencia de acciones del nuevo nodo.
	nodoNuevo.secuencia.push_back(accion);

	return nodoNuevo;
}

// list<Action> ComportamientoJugador::encuentraCaminoAStarSonambulo(const Estado &origen, const ubicacion &destino)
// {
// 	priority_queue<Nodo *> planAStar;

// 	// COMPLETAR

// 	list<Action> ruta;

// 	// Construye la lista de nodos en el orden correcto
// 	while (!planAStar.empty())
// 	{
// 		Nodo *nodoActual = planAStar.top();
// 		ruta.push_front(nodoActual);
// 		planAStar.pop();
// 	}

// 	return ruta;
// }

// list<Action> ComportamientoJugador::maximizarPuntuacion(const Estado &origen, const ubicacion &destino)
// {
// 	priority_queue<Nodo *> planMaxPuntuacion;

// 	// COMPLETAR

// 	list<Action> ruta;

// 	// Construye la lista de nodos en el orden correcto
// 	while (!planMaxPuntuacion.empty())
// 	{
// 		Nodo *nodoActual = planMaxPuntuacion.top();
// 		ruta.push_front(nodoActual);
// 		planMaxPuntuacion.pop();
// 	}

// 	return ruta;
// }

bool ComportamientoJugador::esDestinoJugador(const Estado &origen, const ubicacion &destino)
{
	return (origen.jugador.f == destino.f && origen.jugador.c == destino.c);
}

bool ComportamientoJugador::esDestinoSonambulo(const Estado &origen, const ubicacion &destino)
{
	return (origen.sonambulo.f == destino.f && origen.sonambulo.c == destino.c);
}

// Action ComportamientoJugador::mejorMovimiento(const Estado &estado, const vector<vector<Celda>> &mapa, Entidad agente)
// {
// 	int minCosto = numeric_limits<int>::max();
// 	Action mejorAccion = actIDLE;

// 	// Definir las direcciones posibles según el agente
// 	vector<Orientacion> direcciones;
// 	if (agente == Entidad::Jugador)
// 	{
// 		direcciones = {norte, este, sur, oeste};
// 	}
// 	else if (agente == Entidad::Sonambulo)
// 	{
// 		direcciones = {norte, noreste, este, sureste, sur, suroeste, oeste, noroeste};
// 	}

// 	// Recorrer las direcciones posibles y encontrar la de menor coste
// 	for (const auto &direccion : direcciones)
// 	{
// 		ubicacion posActual = {estado.jugador.f, estado.jugador.c, direccion};
// 		ubicacion siguientePos = siguienteCasilla(posActual);

// 		// Verificar que la siguiente casilla es válida (no fuera del mapa)
// 		if (siguientePos.f == posActual.f && siguientePos.c == posActual.c)
// 		{
// 			continue;
// 		}

// 		int costo = calcularCostoBateria(actFORWARD, mapa[siguientePos.f][siguientePos.c].terreno);
// 		if (costo < minCosto)
// 		{
// 			minCosto = costo;
// 			mejorAccion = actFORWARD;
// 		}
// 	}

// 	return mejorAccion;
// }

void ComportamientoJugador::actualizarMapaCeldas()
{
	for (int i = 0; i < mapaResultado.size(); ++i)
	{
		for (int j = 0; j < mapaResultado[0].size(); ++j)
		{
			mapaCeldas[i][j].terreno = charATerreno(mapaResultado[i][j]);
			mapaCeldas[i][j].superficie = charAEntidad(mapaEntidades[i][j]);
		}
	}
}

const vector<vector<Celda>> &ComportamientoJugador::getMapaCeldas() const
{
	return mapaCeldas;
}

vector<vector<Celda>> &ComportamientoJugador::getMapaCeldas()
{
	return mapaCeldas;
}

void ComportamientoJugador::actualizaVisionJugador(const Estado &estado)
{
	if (nivel != 4)
	{
		const int DIM = 4; // dimension de la vision
		int indice = 0;

		int f = estado.jugador.f;
		int c = estado.jugador.c;

		vision[indice].terreno = charATerreno(mapaResultado[f][c]);
		vision[indice].superficie = charAEntidad(mapaEntidades[f][c]);

		indice++;

		sonambuloEnVision = false;

		switch (estado.jugador.brujula)
		{
		case norte: // vision Norte
			for (int i = 1; i <= DIM - 1; i++)
			{
				for (int j = -i; j <= i; j++)
				{
					int f = estado.jugador.f - i;
					int c = estado.jugador.c + j;

					vision[indice].terreno = charATerreno(mapaResultado[f][c]);
					vision[indice].superficie = charAEntidad(mapaEntidades[f][c]);

					if (f == estado.sonambulo.f && c == estado.sonambulo.c)
						sonambuloEnVision = true;

					indice++;
				}
			}
			break;
		case noreste: // vision Noreste
			for (int i = 1; i <= DIM - 1; i++)
			{
				for (int j = -i; j <= i; j++)
				{
					int f = estado.jugador.f - i;
					int c = estado.jugador.c + i;

					if (j < 0)
					{
						c += j;
					}
					else if (j > 0)
					{
						f += j;
					}

					vision[indice].terreno = charATerreno(mapaResultado[f][c]);
					vision[indice].superficie = charAEntidad(mapaEntidades[f][c]);

					if (f == estado.sonambulo.f && c == estado.sonambulo.c)
						sonambuloEnVision = true;

					indice++;
				}
			}
			break;
		case este: // vision Este
			for (int i = 1; i <= DIM - 1; i++)
			{
				for (int j = -i; j <= i; j++)
				{
					int f = estado.jugador.f + j;
					int c = estado.jugador.c + i;

					vision[indice].terreno = charATerreno(mapaResultado[f][c]);
					vision[indice].superficie = charAEntidad(mapaEntidades[f][c]);

					if (f == estado.sonambulo.f && c == estado.sonambulo.c)
						sonambuloEnVision = true;

					indice++;
				}
			}
			break;
		case sureste: // vision Sureste
			for (int i = 1; i <= DIM - 1; i++)
			{
				for (int j = -i; j <= i; j++)
				{
					int f = estado.jugador.f + i;
					int c = estado.jugador.c + i;

					if (j < 0)
					{
						f += j;
					}
					else if (j > 0)
					{
						c -= j;
					}

					vision[indice].terreno = charATerreno(mapaResultado[f][c]);
					vision[indice].superficie = charAEntidad(mapaEntidades[f][c]);

					if (f == estado.sonambulo.f && c == estado.sonambulo.c)
						sonambuloEnVision = true;

					indice++;
				}
			}
			break;
		case sur: // vision Sur
			for (int i = 1; i <= DIM - 1; i++)
			{
				for (int j = -i; j <= i; j++)
				{
					int f = estado.jugador.f + i;
					int c = estado.jugador.c - j;

					vision[indice].terreno = charATerreno(mapaResultado[f][c]);
					vision[indice].superficie = charAEntidad(mapaEntidades[f][c]);

					if (f == estado.sonambulo.f && c == estado.sonambulo.c)
						sonambuloEnVision = true;

					indice++;
				}
			}
			break;
		case suroeste: // vision Suroeste
			for (int i = 1; i <= DIM - 1; i++)
			{
				for (int j = -i; j <= i; j++)
				{
					int f = estado.jugador.f + i;
					int c = estado.jugador.c - i;

					if (j < 0)
					{
						c -= j;
					}
					else if (j > 0)
					{
						f -= j;
					}

					vision[indice].terreno = charATerreno(mapaResultado[f][c]);
					vision[indice].superficie = charAEntidad(mapaEntidades[f][c]);

					if (f == estado.sonambulo.f && c == estado.sonambulo.c)
						sonambuloEnVision = true;

					indice++;
				}
			}
			break;
		case oeste: // vision Oeste
			for (int i = 1; i <= DIM - 1; i++)
			{
				for (int j = -i; j <= i; j++)
				{
					int f = estado.jugador.f - j;
					int c = estado.jugador.c - i;

					vision[indice].terreno = charATerreno(mapaResultado[f][c]);
					vision[indice].superficie = charAEntidad(mapaEntidades[f][c]);

					if (f == estado.sonambulo.f && c == estado.sonambulo.c)
						sonambuloEnVision = true;

					indice++;
				}
			}
			break;
		case noroeste: // vision Noroeste
			for (int i = 1; i <= DIM - 1; i++)
			{
				for (int j = -i; j <= i; j++)
				{
					int f = estado.jugador.f - i;
					int c = estado.jugador.c - i;

					if (j < 0)
					{
						f -= j;
					}
					else if (j > 0)
					{
						c += j;
					}

					vision[indice].terreno = charATerreno(mapaResultado[f][c]);
					vision[indice].superficie = charAEntidad(mapaEntidades[f][c]);

					if (f == estado.sonambulo.f && c == estado.sonambulo.c)
						sonambuloEnVision = true;

					indice++;
				}
			}
			break;
		}
	}
}

void ComportamientoJugador::actualizaMapaResultVisionJugador(const Sensores &sensores)
{
	const int DIM = 4; // dimension de la vision
	int indice = 0;

	int f = estadoActual.jugador.f;
	int c = estadoActual.jugador.c;

	if (mapaResultado[f][c] == '?')
		mapaResultado[f][c] = sensores.terreno[indice];

	switch (estadoActual.jugador.brujula)
	{
	case norte: // vision Norte
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int f = estadoActual.jugador.f - i;
				int c = estadoActual.jugador.c + j;

				if (f >= 0 && f < mapaAux.size() && c >= 0 && c < mapaAux[0].size())
				{
					if (mapaResultado[f][c] == '?')
						mapaResultado[f][c] = sensores.terreno[indice];

					indice++;
				}
			}
		}
		break;
	case noreste: // vision Noreste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int f = estadoActual.jugador.f - i;
				int c = estadoActual.jugador.c + i;

				if (j < 0)
				{
					c += j;
				}
				else if (j > 0)
				{
					f += j;
				}

				if (f >= 0 && f < mapaAux.size() && c >= 0 && c < mapaAux[0].size())
				{
					if (mapaResultado[f][c] == '?')
						mapaResultado[f][c] = sensores.terreno[indice];

					indice++;
				}
			}
		}
		break;
	case este: // vision Este
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int f = estadoActual.jugador.f + j;
				int c = estadoActual.jugador.c + i;

				if (f >= 0 && f < mapaAux.size() && c >= 0 && c < mapaAux[0].size())
				{
					if (mapaResultado[f][c] == '?')
						mapaResultado[f][c] = sensores.terreno[indice];

					indice++;
				}
			}
		}
		break;
	case sureste: // vision Sureste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int f = estadoActual.jugador.f + i;
				int c = estadoActual.jugador.c + i;

				if (j < 0)
				{
					f += j;
				}
				else if (j > 0)
				{
					c -= j;
				}

				if (f >= 0 && f < mapaAux.size() && c >= 0 && c < mapaAux[0].size())
				{
					if (mapaResultado[f][c] == '?')
						mapaResultado[f][c] = sensores.terreno[indice];

					indice++;
				}
			}
		}
		break;
	case sur: // vision Sur
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int f = estadoActual.jugador.f + i;
				int c = estadoActual.jugador.c - j;

				if (f >= 0 && f < mapaAux.size() && c >= 0 && c < mapaAux[0].size())
				{
					if (mapaResultado[f][c] == '?')
						mapaResultado[f][c] = sensores.terreno[indice];

					indice++;
				}
			}
		}
		break;
	case suroeste: // vision Suroeste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int f = estadoActual.jugador.f + i;
				int c = estadoActual.jugador.c - i;

				if (j < 0)
				{
					c -= j;
				}
				else if (j > 0)
				{
					f -= j;
				}

				if (f >= 0 && f < mapaAux.size() && c >= 0 && c < mapaAux[0].size())
				{
					if (mapaResultado[f][c] == '?')
						mapaResultado[f][c] = sensores.terreno[indice];

					indice++;
				}
			}
		}
		break;
	case oeste: // vision Oeste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int f = estadoActual.jugador.f - j;
				int c = estadoActual.jugador.c - i;

				if (f >= 0 && f < mapaAux.size() && c >= 0 && c < mapaAux[0].size())
				{
					if (mapaResultado[f][c] == '?')
						mapaResultado[f][c] = sensores.terreno[indice];

					indice++;
				}
			}
		}
		break;
	case noroeste: // vision Noroeste
		for (int i = 1; i <= DIM - 1; i++)
		{
			for (int j = -i; j <= i; j++)
			{
				int f = estadoActual.jugador.f - i;
				int c = estadoActual.jugador.c - i;

				if (j < 0)
				{
					f -= j;
				}
				else if (j > 0)
				{
					c += j;
				}

				if (f >= 0 && f < mapaAux.size() && c >= 0 && c < mapaAux[0].size())
				{
					if (mapaResultado[f][c] == '?')
						mapaResultado[f][c] = sensores.terreno[indice];

					indice++;
				}
			}
		}
		break;
	}
}

vector<vector<Celda>> ComportamientoJugador::getAreaLocalJugador(const vector<vector<Celda>> &mapa, const Estado &estado)
{
	int coordenadaXActual = estado.jugador.f;
	int coordenadaYActual = estado.jugador.c;

	int dimArea = 1;
	int area = 2 * dimArea + 1;

	vector<vector<Celda>> areaLocal(area, vector<Celda>(area));

	for (int i = -dimArea; i <= dimArea; ++i)
	{
		for (int j = -dimArea; j <= dimArea; ++j)
		{
			int x = coordenadaXActual + i;
			int y = coordenadaYActual + j;

			// Verificar que las coordenadas estén dentro del mapa
			if (x >= 0 && x < mapa.size() && y >= 0 && y < mapa.size())
				areaLocal[i + dimArea][j + dimArea] = mapa[x][y];
		}
	}
	return areaLocal;
}

vector<vector<Celda>> ComportamientoJugador::getAreaLocalSonambulo(const vector<vector<Celda>> &mapa, const Estado &estado)
{
	int coordenadaXActual = estado.sonambulo.f;
	int coordenadaYActual = estado.sonambulo.c;

	int dimArea = 1;
	int area = 2 * dimArea + 1;

	vector<vector<Celda>> areaLocal(area, vector<Celda>(area));

	for (int i = -dimArea; i <= dimArea; ++i)
	{
		for (int j = -dimArea; j <= dimArea; ++j)
		{
			int x = coordenadaXActual + i;
			int y = coordenadaYActual + j;

			// Verificar que las coordenadas estén dentro del mapa
			if (x >= 0 && x < mapa.size() && y >= 0 && y < mapa.size())
				areaLocal[i + dimArea][j + dimArea] = mapa[x][y];
		}
	}
	return areaLocal;
}

int ComportamientoJugador::calcularCostoBateria(Action accion, unsigned char tipoCasilla)
{
	int costo = 0;

	switch (accion)
	{
	case actFORWARD:
	case actSON_FORWARD:
		switch (tipoCasilla)
		{
		case Terreno::Agua:
			costo = tieneBikini ? 10 : 100;
			break;
		case Terreno::Bosque:
			costo = tieneZapatillas ? 15 : 50;
			break;
		case Terreno::SueloArenoso:
			costo = 2;
			break;
		default:
			costo = 1;
			break;
		}
		break;
	case actTURN_L:
	case actTURN_R:
		switch (tipoCasilla)
		{
		case Terreno::Agua:
			costo = tieneBikini ? 5 : 25;
			break;
		case Terreno::Bosque:
			costo = tieneZapatillas ? 1 : 5;
			break;
		case Terreno::SueloArenoso:
			costo = 2;
			break;
		default:
			costo = 1;
			break;
		}
		break;
	case actSON_TURN_SL:
	case actSON_TURN_SR:
		switch (tipoCasilla)
		{
		case Terreno::Agua:
			costo = tieneBikini ? 2 : 7;
			break;
		case Terreno::Bosque:
			costo = tieneZapatillas ? 1 : 3;
			break;
		case Terreno::SueloArenoso:
			costo = 1;
			break;
		default:
			costo = 1;
			break;
		}
		break;
	case actWHEREIS:
		costo = 200;
		break;
	case actIDLE:
		costo = 0;
		break;
	}

	return costo;
}

int ComportamientoJugador::calcularCostoBateria(Action accion, unsigned char tipoCasilla, const Objeto &objetoJugador)
{
	int costo = 0;

	switch (accion)
	{
	case actFORWARD:
	case actSON_FORWARD:
		switch (tipoCasilla)
		{
		case Terreno::Agua:
			costo = objetoJugador.bikini ? 10 : 100;
			break;
		case Terreno::Bosque:
			costo = objetoJugador.zapatillas ? 15 : 50;
			break;
		case Terreno::SueloArenoso:
			costo = 2;
			break;
		default:
			costo = 1;
			break;
		}
		break;
	case actTURN_L:
	case actTURN_R:
		switch (tipoCasilla)
		{
		case Terreno::Agua:
			costo = objetoJugador.bikini ? 5 : 25;
			break;
		case Terreno::Bosque:
			costo = objetoJugador.zapatillas ? 1 : 5;
			break;
		case Terreno::SueloArenoso:
			costo = 2;
			break;
		default:
			costo = 1;
			break;
		}
		break;
	case actSON_TURN_SL:
	case actSON_TURN_SR:
		switch (tipoCasilla)
		{
		case Terreno::Agua:
			costo = objetoJugador.bikini ? 2 : 7;
			break;
		case Terreno::Bosque:
			costo = objetoJugador.zapatillas ? 1 : 3;
			break;
		case Terreno::SueloArenoso:
			costo = 1;
			break;
		default:
			costo = 1;
			break;
		}
		break;
	case actWHEREIS:
		costo = 200;
		break;
	case actIDLE:
		costo = 0;
		break;
	}

	return costo;
}

int ComportamientoJugador::interact(Action accion, int valor)
{
	return false;
}

//-----------------------------------------------------------------------------------------------//

void ComportamientoJugador::debug(bool imprimir) const
{
	if (imprimir)
	{
		cout << "ESTADO actual del JUGADOR:" << endl;
		cout << "Fila: " << estadoActual.jugador.f << " Columna: " << estadoActual.jugador.c << " Orientacion: " << toString(estadoActual.jugador.brujula) << endl
			 << endl;
		cout << "ESTADO actual del SONAMBULO:" << endl;
		cout << "Fila: " << estadoActual.sonambulo.f << " Columna: " << estadoActual.sonambulo.c << " Orientacion: " << toString(estadoActual.sonambulo.brujula) << endl
			 << endl;
		cout << "Nivel: " << nivel << endl;
		cout << "Bateria: " << bateria << endl;
		cout << "Primera iteracion: " << (primeraIteracion ? "true" : "false") << endl;
		cout << "Tiempo: " << tiempo << endl
			 << endl;
		cout << "Tiene BIKINI: " << (tieneBikini ? "true" : "false") << endl;
		cout << "Tiene ZAPATILLAS: " << (tieneZapatillas ? "true" : "false") << endl
			 << endl;
		cout << "Ultima Accion del JUGADOR: " << toString(ultimaAccionJugador) << endl;
		cout << "Ultima Accion del SONAMBULO: " << toString(ultimaAccionSonambulo) << endl
			 << endl;
		cout << "Hay plan: " << (hayPlan ? "true" : "false") << endl
			 << endl;
	}
}

string ComportamientoJugador::toString(Orientacion orientacion) const
{
	switch (orientacion)
	{
	case norte:
		return "norte";
	case noreste:
		return "noreste";
	case este:
		return "este";
	case sureste:
		return "sureste";
	case sur:
		return "sur";
	case suroeste:
		return "suroeste";
	case oeste:
		return "oeste";
	case noroeste:
		return "noroeste";
	default:
		return "desconocida";
	}
}

string ComportamientoJugador::toString(Action accion) const
{
	switch (accion)
	{
	case actFORWARD:
		return "actFORWARD";
	case actTURN_L:
		return "actTURN_L";
	case actTURN_R:
		return "actTURN_R";
	case actTURN_SL:
		return "actTURN_SL";
	case actTURN_SR:
		return "actTURN_SR";
	case actWHEREIS:
		return "actWHEREIS";
	case actSON_FORWARD:
		return "actSON_FORWARD";
	case actSON_TURN_SL:
		return "actSON_TURN_SL";
	case actSON_TURN_SR:
		return "actSON_TURN_SR";
	case actIDLE:
		return "actIDLE";
	default:
		return "unknown";
	}
}

//-----------------------------------------------------------------------------------------------//