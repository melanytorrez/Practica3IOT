# -*- coding: utf-8 -*-
import logging
import json
import boto3

import ask_sdk_core.utils as ask_utils
from ask_sdk_core.skill_builder import SkillBuilder
from ask_sdk_core.dispatch_components import AbstractRequestHandler
from ask_sdk_core.dispatch_components import AbstractExceptionHandler
from ask_sdk_core.handler_input import HandlerInput
from ask_sdk_model import Response

# --- CONFIGURACIÓN DE TU PROYECTO ---
THING_NAME = "MiCasa" 
# ------------------------------------

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)

iot_client = boto3.client('iot-data')

# --- HANDLERS CON LA SESIÓN ABIERTA ---

class EncenderLuzIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("EncenderLuzIntent")(handler_input)

    def handle(self, handler_input):
        speak_output = "Ok, he encendido la luz. ¿Puedo ayudarte en algo más?"
        reprompt_text = "Dime qué más quieres hacer."
        payload = {"state": {"desired": {"ledState": "ON"}}}
        
        iot_client.update_thing_shadow(thingName=THING_NAME, payload=json.dumps(payload))

        # Añadimos .ask() para mantener la sesión abierta
        return handler_input.response_builder.speak(speak_output).ask(reprompt_text).response

class ApagarLuzIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("ApagarLuzIntent")(handler_input)

    def handle(self, handler_input):
        speak_output = "Listo, luz apagada. ¿Necesitas otra cosa?"
        reprompt_text = "Puedes pedirme que cierre la puerta."
        payload = {"state": {"desired": {"ledState": "OFF"}}}
        
        iot_client.update_thing_shadow(thingName=THING_NAME, payload=json.dumps(payload))

        # Añadimos .ask()
        return handler_input.response_builder.speak(speak_output).ask(reprompt_text).response

class AbrirPuertaIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("AbrirPuertaIntent")(handler_input)

    def handle(self, handler_input):
        speak_output = "Ok, abriendo la puerta. ¿Algo más?"
        reprompt_text = "¿Necesitas algo más?"
        payload = {"state": {"desired": {"doorState": "OPEN"}}}

        iot_client.update_thing_shadow(thingName=THING_NAME, payload=json.dumps(payload))

        # Añadimos .ask()
        return handler_input.response_builder.speak(speak_output).ask(reprompt_text).response

class CerrarPuertaIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("CerrarPuertaIntent")(handler_input)

    def handle(self, handler_input):
        speak_output = "Entendido, cerrando la puerta. ¿Te ayudo con otra cosa?"
        reprompt_text = "Puedes pedirme que encienda la luz, por ejemplo."
        payload = {"state": {"desired": {"doorState": "CLOSED"}}}
        
        iot_client.update_thing_shadow(thingName=THING_NAME, payload=json.dumps(payload))

        # Añadimos .ask()
        return handler_input.response_builder.speak(speak_output).ask(reprompt_text).response

class EstadoSensorIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("EstadoSensorIntent")(handler_input)

    def handle(self, handler_input):
        try:
            response = iot_client.get_thing_shadow(thingName=THING_NAME)
            shadow_data = json.loads(response['payload'].read().decode('utf-8'))
            estado_movimiento = shadow_data['state']['reported']['motion']

            if estado_movimiento == "DETECTED":
                speak_output = "Se ha detectado movimiento en la habitación. ¿Necesitas algo más?"
            else: # Asumimos NOT_DETECTED
                speak_output = "Todo está tranquilo, no se detecta movimiento. ¿Te ayudo con otra cosa?"
        except Exception as e:
            logger.error(f"Error al obtener el shadow: {e}")
            speak_output = "Lo siento, tuve problemas para comunicarme con el sensor."

        reprompt_text = "Puedes pedirme que controle la luz o la puerta."
        
        # Con .ask(), la sesión se mantendrá abierta.
        return handler_input.response_builder.speak(speak_output).ask(reprompt_text).response

# --- HANDLERS ESTÁNDAR DE ALEXA ---
# (No se necesita modificarlos)
class LaunchRequestHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_request_type("LaunchRequest")(handler_input)
    def handle(self, handler_input):
        speak_output = "Bienvenido al control de tu casa inteligente. Puedes pedirme que encienda la luz, abra la puerta o que te diga si hay movimiento."
        return handler_input.response_builder.speak(speak_output).ask(speak_output).response

class HelpIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("AMAZON.HelpIntent")(handler_input)
    def handle(self, handler_input):
        speak_output = "Puedes darme órdenes como 'enciende la luz', 'cierra la puerta', o preguntar 'hay movimiento'."
        return handler_input.response_builder.speak(speak_output).ask(speak_output).response

class CancelOrStopIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return (ask_utils.is_intent_name("AMAZON.CancelIntent")(handler_input) or
                ask_utils.is_intent_name("AMAZON.StopIntent")(handler_input))
    def handle(self, handler_input):
        speak_output = "¡Hasta pronto!"
        return handler_input.response_builder.speak(speak_output).response

class FallbackIntentHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_intent_name("AMAZON.FallbackIntent")(handler_input)
    def handle(self, handler_input):
        speech = "Lo siento, no te he entendido. Puedes pedirme que controle las luces o la puerta."
        reprompt = "No entendí tu orden. ¿Qué quieres hacer?"
        return handler_input.response_builder.speak(speech).ask(reprompt).response

class SessionEndedRequestHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_request_type("SessionEndedRequest")(handler_input)
    def handle(self, handler_input):
        return handler_input.response_builder.response

class IntentReflectorHandler(AbstractRequestHandler):
    def can_handle(self, handler_input):
        return ask_utils.is_request_type("IntentRequest")(handler_input)
    def handle(self, handler_input):
        intent_name = ask_utils.get_intent_name(handler_input)
        speak_output = "Acabas de activar la intención " + intent_name + "."
        return handler_input.response_builder.speak(speak_output).response

class CatchAllExceptionHandler(AbstractExceptionHandler):
    def can_handle(self, handler_input, exception):
        return True
    def handle(self, handler_input, exception):
        logger.error(exception, exc_info=True)
        speak_output = "Lo siento, tuve un problema al procesar tu solicitud. Inténtalo de nuevo."
        return handler_input.response_builder.speak(speak_output).ask(speak_output).response

# --- CONSTRUCCIÓN DE LA SKILL ---
sb = SkillBuilder()
sb.add_request_handler(LaunchRequestHandler())
sb.add_request_handler(EncenderLuzIntentHandler())
sb.add_request_handler(ApagarLuzIntentHandler())
sb.add_request_handler(AbrirPuertaIntentHandler())
sb.add_request_handler(CerrarPuertaIntentHandler())
sb.add_request_handler(EstadoSensorIntentHandler())
sb.add_request_handler(HelpIntentHandler())
sb.add_request_handler(CancelOrStopIntentHandler())
sb.add_request_handler(FallbackIntentHandler())
sb.add_request_handler(SessionEndedRequestHandler())
sb.add_request_handler(IntentReflectorHandler()) 
sb.add_exception_handler(CatchAllExceptionHandler())
lambda_handler = sb.lambda_handler()