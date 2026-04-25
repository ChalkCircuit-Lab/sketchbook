import { type Static, Type } from '@sinclair/typebox';
import { CONFIG_DEFAULTS } from '../utils/constants/constants';

const EnvSchema = Type.Object({
  NODE_ENV: Type.String({ default: CONFIG_DEFAULTS.ENV }),
  APP_PORT: Type.Number({ default: CONFIG_DEFAULTS.PORT }),
  MONGO_URL: Type.String({ default: CONFIG_DEFAULTS.MONGO_URL }),
  MONGO_DB_NAME: Type.String({ default: CONFIG_DEFAULTS.MONGO_DB_NAME }),
  GOOGLE_CLOUD_PROJECT: Type.Optional(Type.String()),
  MOVIE_EVENTS_TOPIC: Type.String({ default: CONFIG_DEFAULTS.MOVIE_EVENTS_TOPIC })
});

type EnvSchemaType = Static<typeof EnvSchema>;

export { EnvSchema, type EnvSchemaType };
