class AgentOutput:
    def __init__(self, move_command, message):
        self.move_command = move_command
        self.message = message

    def get_move_command(self):
        return self.move_command
    
    def set_move_command(self, move_command):
        self.move_command = move_command

    def get_message(self):
        return self.message
    
    def set_message(self, message):
        self.message = message